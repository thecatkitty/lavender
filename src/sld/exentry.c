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

#define LINE_WIDTH 80

extern const char StrSldEnterSerial[];
extern const char StrSldSerial[];

typedef struct
{
    crg_stream *CrgContext;
    uint32_t    Crc;
    uint32_t   *Local;
} SLD_KEY_VALIDATION;

static gfx_dimensions s_Screen;

static uint16_t s_Accumulator = 0;

static int
SldExecuteText(sld_entry *sld);

static int
SldExecuteBitmap(sld_entry *sld);

static int
SldExecuteRectangle(sld_entry *sld);

static int
SldExecutePlay(sld_entry *sld);

static int
SldExecuteScriptCall(sld_entry *sld);

static hasset
SldFindBestBitmap(char *pattern);

static bool
SldIsXorKeyValid(const uint8_t *key, int keyLength, void *context);

static bool
SldPromptVolumeSerialNumber(char *sn);

static bool
SldIsVolumeSerialNumberValid(const char *sn);

int
sld_execute_entry(sld_entry *sld)
{
    if (0 == s_Screen.width)
    {
        gfx_get_screen_dimensions(&s_Screen);
    }

    pal_sleep(sld->delay);

    switch (sld->type)
    {
    case SLD_TYPE_BLANK:
        return 0;
    case SLD_TYPE_LABEL:
        return 0;
    case SLD_TYPE_TEXT:
        return SldExecuteText(sld);
    case SLD_TYPE_BITMAP:
        return SldExecuteBitmap(sld);
    case SLD_TYPE_RECT:
    case SLD_TYPE_RECTF:
        return SldExecuteRectangle(sld);
    case SLD_TYPE_PLAY:
        return SldExecutePlay(sld);
    case SLD_TYPE_WAITKEY:
        s_Accumulator = bios_get_keystroke() >> 8;
        return 0;
    case SLD_TYPE_JUMP:
        return INT_MAX;
    case SLD_TYPE_JUMPE:
        return (s_Accumulator == sld->posy) ? INT_MAX : 0;
    case SLD_TYPE_CALL:
        return SldExecuteScriptCall(sld);
    }

    ERR(SLD_UNKNOWN_TYPE);
}

int
SldExecuteText(sld_entry *sld)
{
    uint16_t x, y = sld->posy;

    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (LINE_WIDTH - sld->length) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = LINE_WIDTH - sld->length;
        break;
    default:
        x = sld->posx;
    }

    return gfx_draw_text(sld->content, x, y) ? 0 : ERR_KER_UNSUPPORTED;
}

int
SldExecuteBitmap(sld_entry *sld)
{
    int status;

    hasset bitmap = SldFindBestBitmap(sld->content);
    if (NULL == bitmap)
    {
        ERR(KER_NOT_FOUND);
    }

    gfx_bitmap bm;
    if (!pbm_load_bitmap(&bm, bitmap))
    {
        ERR(KER_NOT_FOUND);
    }

    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (LINE_WIDTH - bm.opl) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = LINE_WIDTH - bm.opl;
        break;
    default:
        x = sld->posx;
    }

    status = gfx_draw_bitmap(&bm, x, y);

    pal_close_asset(bitmap);
    return status ? 0 : ERR_KER_UNSUPPORTED;
}

int
SldExecuteRectangle(sld_entry *sld)
{
    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (s_Screen.width - sld->shape.dimensions.width) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = s_Screen.height - sld->shape.dimensions.height;
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

int
SldExecutePlay(sld_entry *sld)
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

int
SldExecuteScriptCall(sld_entry *sld)
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
    s_Accumulator = 0;

    // Run if stored as plain text
    if (SLD_METHOD_STORE == sld->script_call.method)
    {
        return sld_run_script(data, size);
    }

    crg_stream         ctx;
    uint8_t            key[sizeof(uint64_t)];
    SLD_KEY_VALIDATION context = {&ctx, sld->script_call.crc32, NULL};
    bool               invalid = false;

    crg_prepare(&ctx, CRG_XOR, data, size, key, 6);

    memset(key, 0, sizeof(key));
    switch (sld->script_call.parameter)
    {
    case SLD_PARAMETER_XOR48_INLINE:
        *(uint64_t *)&key = rstrtoull(sld->script_call.data, 16);
        break;
    case SLD_PARAMETER_XOR48_PROMPT:
        invalid =
            !__sld_prompt_passcode(key, 6, 16, SldIsXorKeyValid, &context);
        break;
    case SLD_PARAMETER_XOR48_SPLIT: {
        uint32_t local = strtoul(sld->script_call.data, NULL, 16);
        context.Local = &local;
        if (!__sld_prompt_passcode(key, 3, 10, SldIsXorKeyValid, &context))
        {
            invalid = true;
            break;
        }

        *(uint64_t *)&key = crg_combine_key(local, *(const uint32_t *)&key);
        context.Local = NULL;
        break;
    }
    case SLD_PARAMETER_XOR48_DISKID: {
        uint32_t medium_id = pal_get_medium_id(sld->script_call.data);
        if (0 == medium_id)
        {
            char sn[10];
            if (!SldPromptVolumeSerialNumber(sn))
            {
                invalid = true;
                break;
            }

            uint32_t highPart = strtoul(sn, NULL, 16);
            uint32_t lowPart = strtoul(sn + 5, NULL, 16);
            medium_id = (highPart << 16) | lowPart;
        }

        context.Local = &medium_id;
        if (!__sld_prompt_passcode(key, 3, 10, SldIsXorKeyValid, &context))
        {
            invalid = true;
            break;
        }

        *(uint64_t *)&key = crg_combine_key(medium_id, *(const uint32_t *)&key);
        context.Local = NULL;
        break;
    }
    default:
        invalid = true;
        break;
    }

    // Check the key
    if (invalid || !SldIsXorKeyValid((const uint8_t *)&key, 6, &context))
    {
        s_Accumulator = UINT16_MAX;
        return 0;
    }

    crg_decrypt(&ctx, data);
    sld->script_call.method = SLD_METHOD_STORE;
    status = sld_run_script(data, size);

    pal_close_asset(script);
    return status;
}

hasset
SldFindBestBitmap(char *pattern)
{
    const char *hex = "0123456789ABCDEF";
    char       *placeholder = strstr(pattern, "<>");
    if (NULL == placeholder)
    {
        return pal_open_asset(pattern, O_RDONLY);
    }

    int par = (int)gfx_get_pixel_aspect();
    int offset = 0;

    while ((0 <= (par + offset)) || (255 >= (par + offset)))
    {
        if ((0 <= (par + offset)) && (255 >= (par + offset)))
        {
            placeholder[0] = hex[(par + offset) / 16];
            placeholder[1] = hex[(par + offset) % 16];
            hasset asset = pal_open_asset(pattern, O_RDONLY);
            if (NULL != asset)
            {
                return asset;
            }
        }

        if (0 <= offset)
        {
            offset++;
        }
        offset = -offset;
    }

    return NULL;
}

bool
SldIsXorKeyValid(const uint8_t *key, int keyLength, void *context)
{
    SLD_KEY_VALIDATION *keyValidation = (SLD_KEY_VALIDATION *)context;

    if (!keyValidation->Local)
    {
        return crg_validate(keyValidation->CrgContext, keyValidation->Crc);
    }

    // 48-bit split key
    uint64_t fullKey =
        crg_combine_key(*keyValidation->Local, *(const uint32_t *)key);
    keyValidation->CrgContext->key = (uint8_t *)&fullKey;
    return crg_validate(keyValidation->CrgContext, keyValidation->Crc);
}

bool
SldPromptVolumeSerialNumber(char *sn)
{
    return 0 != dlg_prompt(StrSldEnterSerial, StrSldSerial, sn, 9,
                           SldIsVolumeSerialNumberValid);
}

bool
SldIsVolumeSerialNumberValid(const char *sn)
{
    if (9 != strlen(sn))
    {
        return false;
    }

    for (int i = 0; i < 9; i++)
    {
        if (4 == i)
        {
            if ('-' != sn[i])
            {
                return false;
            }
        }
        else
        {
            if (!isxdigit(sn[i]))
            {
                return false;
            }
        }
    }

    return true;
}
