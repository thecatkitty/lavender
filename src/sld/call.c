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

static bool
_acquire_xor_key(uint64_t   *key,
                 crg_stream *crs,
                 uint32_t    crc32,
                 uint16_t    parameter,
                 char       *data)
{
    _key_validation context = {crs, crc32, NULL};
    bool            invalid = false;
    uint8_t        *keybs = (uint8_t *)key;

    *key = 0;

    switch (parameter)
    {
    case SLD_PARAMETER_XOR48_INLINE:
        *key = rstrtoull(data, 16);
        break;
    case SLD_PARAMETER_XOR48_PROMPT:
        invalid = !_prompt_passcode(keybs, 6, 16, _validate_xor_key, &context);
        break;
    case SLD_PARAMETER_XOR48_SPLIT: {
        uint32_t local = strtoul(data, NULL, 16);
        context.local = &local;
        if (!_prompt_passcode(keybs, 3, 10, _validate_xor_key, &context))
        {
            invalid = true;
            break;
        }

        *key = crg_combine_key(local, *(const uint32_t *)key);
        context.local = NULL;
        break;
    }
    case SLD_PARAMETER_XOR48_DISKID: {
        uint32_t medium_id = pal_get_medium_id(data);
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
        if (!_prompt_passcode(keybs, 3, 10, _validate_xor_key, &context))
        {
            invalid = true;
            break;
        }

        *key = crg_combine_key(medium_id, *(const uint32_t *)key);
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
        return false;
    }

    return true;
}

int
__sld_execute_script_call(sld_entry *sld)
{
    int status = SLD_OK;

    sld_context *script = sld_create_context(sld->script_call.file_name, NULL);
    if (NULL == script)
    {
        __sld_errmsgcpy(sld, IDS_NOEXECCTX);
        return SLD_SYSERR;
    }

    __sld_accumulator = 0;

    // Run if stored as plain text
    if (SLD_METHOD_STORE == sld->script_call.method)
    {
        status = sld_run_script(script);
        sld_close_context(script);
        return status;
    }

    crg_stream crs;
    union {
        uint64_t qw;
        uint8_t  bs[sizeof(uint64_t)];
    } key;

    crg_prepare(&crs, CRG_XOR, script->data, script->size, key.bs, 6);

    if (_acquire_xor_key(&key.qw, &crs, sld->script_call.crc32,
                         sld->script_call.parameter, sld->script_call.data))
    {
        crg_decrypt(&crs, script->data);
        sld->script_call.method = SLD_METHOD_STORE;
        status = sld_run_script(script);
    }

    sld_close_context(script);
    return status;
}

int
__sld_load_script_call(const char *str, sld_entry *out)
{
    const char *cur = str;
    int         length;

    while (isspace(*cur))
    {
        cur++;
    }

    length = 0;
    while (!isspace(*cur))
    {
        if ((sizeof(out->script_call.file_name) - 1) < length)
        {
            __sld_errmsgcpy(out, IDS_LONGNAME);
            return SLD_ARGERR;
        }
        out->script_call.file_name[length] = *(cur++);
        length++;
    }
    out->script_call.file_name[length] = 0;
    out->length = length;

    if (('\r' == *cur) || ('\n' == *cur))
    {
        out->script_call.method = SLD_METHOD_STORE;
        out->script_call.crc32 = 0;
        out->script_call.parameter = 0;
    }
    else
    {
        cur += __sld_loadu(cur, &out->script_call.method);
        out->script_call.crc32 = strtoul(cur, (char **)&cur, 16);
        cur += __sld_loadu(cur, &out->script_call.parameter);
    }

    length = 0;
    while (('\r' != *cur) && ('\n' != *cur))
    {
        if ((sizeof(out->script_call.data) - 1) < length)
        {
            __sld_errmsgcpy(out, IDS_LONGCONTENT);
            return SLD_ARGERR;
        }
        out->script_call.data[length] = *(cur++);
        length++;
    }
    out->script_call.data[length] = 0;

    return cur - str;
}
