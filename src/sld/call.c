
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <crg.h>
#include <dlg.h>
#include <pal.h>

#include "sld_impl.h"

extern const char IDS_ENTERPASS[];
extern const char IDS_ENTERPASS_DESC[];
extern const char IDS_INVALIDKEY[];
extern const char IDS_ENTERDSN[];
extern const char IDS_ENTERDSN_DESC[];

typedef struct
{
    crg_stream *ctx;
    uint32_t    crc32;
    uint32_t   *local;
} _key_validation;

#define _isctypestr(predicate)                                                 \
    {                                                                          \
        if (!*str)                                                             \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        while (*str)                                                           \
        {                                                                      \
            if (!predicate(*str))                                              \
            {                                                                  \
                return false;                                                  \
            }                                                                  \
            str++;                                                             \
        }                                                                      \
                                                                               \
        return true;                                                           \
    }

static bool
_isdigitstr(const char *str)
{
    _isctypestr(isdigit);
}

static bool
_isxdigitstr(const char *str)
{
    _isctypestr(isxdigit);
}

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
_prompt_passcode(uint8_t               *code,
                 int                    code_len,
                 int                    base,
                 sld_passcode_validator validator,
                 void                  *context)
{
    char *buffer = (char *)alloca(code_len * 3 + 1);
    while (true)
    {
        bool (*precheck)(const char *) = NULL;
        switch (base)
        {
        case 10:
            precheck = _isdigitstr;
            break;
        case 16:
            precheck = _isxdigitstr;
            break;
        }

        int length = dlg_prompt(IDS_ENTERPASS, IDS_ENTERPASS_DESC, buffer,
                                code_len * 2, precheck);
        if (0 == length)
        {
            return false;
        }

        if (0 == base)
        {
            memcpy(code, buffer, length);
        }
        else
        {
            uint64_t value = rstrtoull(buffer, base);
            memcpy(code, &value, code_len);
        }

        if (!validator)
        {
            return true;
        }

        if (validator(code, code_len, context))
        {
            return true;
        }

        dlg_alert(IDS_ENTERPASS, IDS_INVALIDKEY);
    }
}

static bool
_prompt_volsn(char *volsn)
{
    return 0 != dlg_prompt(IDS_ENTERDSN, IDS_ENTERDSN_DESC, volsn, 9,
                           _validate_volsn);
}

int
__sld_execute_script_call(sld_entry *sld)
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
    __sld_accumulator = 0;

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
        invalid = !_prompt_passcode(key, 6, 16, _validate_xor_key, &context);
        break;
    case SLD_PARAMETER_XOR48_SPLIT: {
        uint32_t local = strtoul(sld->script_call.data, NULL, 16);
        context.local = &local;
        if (!_prompt_passcode(key, 3, 10, _validate_xor_key, &context))
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
        if (!_prompt_passcode(key, 3, 10, _validate_xor_key, &context))
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
        __sld_accumulator = UINT16_MAX;
        return 0;
    }

    crg_decrypt(&ctx, data);
    sld->script_call.method = SLD_METHOD_STORE;
    status = sld_run_script(data, size);

    pal_close_asset(script);
    return status;
}
