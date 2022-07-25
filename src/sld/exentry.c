#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <crg.h>
#include <dlg.h>
#include <fmt/pbm.h>
#include <gfx.h>
#include <pal.h>
#include <sld.h>
#include <snd.h>

#include "sld_impl.h"

extern const char IDS_ENTERDSN[];
extern const char IDS_ENTERDSN_DESC[];

typedef struct
{
    crg_stream *ctx;
    uint32_t    crc32;
    uint32_t   *local;
} _key_validation;

static gfx_dimensions _screen;

static uint16_t _accumulator = 0;

static bool
_validate_volsn(const char *volsn)
{
    if (9 != strlen(volsn))
    {
        return false;
    }

    for (int i = 0; i < 9; i++)
    {
        if (4 == i)
        {
            if ('-' != volsn[i])
            {
                return false;
            }
        }
        else
        {
            if (!isxdigit(volsn[i]))
            {
                return false;
            }
        }
    }

    return true;
}

static bool
_validate_xor_key(const uint8_t *key, int keyLength, void *context)
{
    _key_validation *validation = (_key_validation *)context;

    if (!validation->local)
    {
        return crg_validate(validation->ctx, validation->crc32);
    }

    // 48-bit split key
    uint64_t fullKey =
        crg_combine_key(*validation->local, *(const uint32_t *)key);
    validation->ctx->key = (uint8_t *)&fullKey;
    return crg_validate(validation->ctx, validation->crc32);
}

static bool
_prompt_volsn(char *volsn)
{
    return 0 != dlg_prompt(IDS_ENTERDSN, IDS_ENTERDSN_DESC, volsn, 9,
                           _validate_volsn);
}

static int
_execute_rectangle(sld_entry *sld)
{
    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (_screen.width - sld->shape.dimensions.width) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = _screen.height - sld->shape.dimensions.height;
        break;
    default:
        x = sld->posx;
    }

    bool (*draw)(gfx_dimensions *, uint16_t, uint16_t, gfx_color);
    draw =
        (SLD_TYPE_RECT == sld->type) ? gfx_draw_rectangle : gfx_fill_rectangle;
    return draw(&sld->shape.dimensions, x, y, sld->shape.color)
               ? 0
               : ERR_KER_UNSUPPORTED;
}

static int
_execute_play(sld_entry *sld)
{
    hasset music = pal_open_asset(sld->content, O_RDONLY);
    if (NULL == music)
    {
        ERR(KER_NOT_FOUND);
    }

    char *data = pal_get_asset_data(music);
    if (NULL == data)
    {
        ERR(KER_NOT_FOUND);
    }

    snd_play(data, pal_get_asset_size(music));
    return 0;
}

static int
_execute_script_call(sld_entry *sld)
{
    int status;

    hasset script = pal_open_asset(sld->script_call.file_name, O_RDWR);
    if (NULL == script)
    {
        ERR(KER_NOT_FOUND);
    }

    char *data = pal_get_asset_data(script);
    if (NULL == data)
    {
        ERR(KER_NOT_FOUND);
    }

    int size = pal_get_asset_size(script);
    _accumulator = 0;

    // Run if stored as plain text
    if (SLD_METHOD_STORE == sld->script_call.method)
    {
        return sld_run_script(data, size);
    }

    crg_stream      ctx;
    uint8_t         key[sizeof(uint64_t)];
    _key_validation context = {&ctx, sld->script_call.crc32, NULL};
    bool            invalid = false;

    crg_prepare(&ctx, CRG_XOR, data, size, key, 6);

    memset(key, 0, sizeof(key));
    switch (sld->script_call.parameter)
    {
    case SLD_PARAMETER_XOR48_INLINE:
        *(uint64_t *)&key = rstrtoull(sld->script_call.data, 16);
        break;
    case SLD_PARAMETER_XOR48_PROMPT:
        invalid =
            !__sld_prompt_passcode(key, 6, 16, _validate_xor_key, &context);
        break;
    case SLD_PARAMETER_XOR48_SPLIT: {
        uint32_t local = strtoul(sld->script_call.data, NULL, 16);
        context.local = &local;
        if (!__sld_prompt_passcode(key, 3, 10, _validate_xor_key, &context))
        {
            invalid = true;
            break;
        }

        *(uint64_t *)&key = crg_combine_key(local, *(const uint32_t *)&key);
        context.local = NULL;
        break;
    }
    case SLD_PARAMETER_XOR48_DISKID: {
        uint32_t medium_id = pal_get_medium_id(sld->script_call.data);
        if (0 == medium_id)
        {
            char volsn[10];
            if (!_prompt_volsn(volsn))
            {
                invalid = true;
                break;
            }

            uint32_t high = strtoul(volsn, NULL, 16);
            uint32_t low = strtoul(volsn + 5, NULL, 16);
            medium_id = (high << 16) | low;
        }

        context.local = &medium_id;
        if (!__sld_prompt_passcode(key, 3, 10, _validate_xor_key, &context))
        {
            invalid = true;
            break;
        }

        *(uint64_t *)&key = crg_combine_key(medium_id, *(const uint32_t *)&key);
        context.local = NULL;
        break;
    }
    default:
        invalid = true;
        break;
    }

    // Check the key
    if (invalid || !_validate_xor_key((const uint8_t *)&key, 6, &context))
    {
        _accumulator = UINT16_MAX;
        return 0;
    }

    crg_decrypt(&ctx, data);
    sld->script_call.method = SLD_METHOD_STORE;
    status = sld_run_script(data, size);

    pal_close_asset(script);
    return status;
}

int
sld_execute_entry(sld_entry *sld)
{
    if (0 == _screen.width)
    {
        gfx_get_screen_dimensions(&_screen);
    }

    pal_sleep(sld->delay);

    switch (sld->type)
    {
    case SLD_TYPE_BLANK:
        return 0;
    case SLD_TYPE_LABEL:
        return 0;
    case SLD_TYPE_TEXT:
        return __sld_execute_text(sld);
    case SLD_TYPE_BITMAP:
        return __sld_execute_bitmap(sld);
    case SLD_TYPE_RECT:
    case SLD_TYPE_RECTF:
        return _execute_rectangle(sld);
    case SLD_TYPE_PLAY:
        return _execute_play(sld);
    case SLD_TYPE_WAITKEY:
        _accumulator = bios_get_keystroke() >> 8;
        return 0;
    case SLD_TYPE_JUMP:
        return INT_MAX;
    case SLD_TYPE_JUMPE:
        return (_accumulator == sld->posy) ? INT_MAX : 0;
    case SLD_TYPE_CALL:
        return _execute_script_call(sld);
    }

    ERR(SLD_UNKNOWN_TYPE);
}
