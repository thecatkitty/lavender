#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <crg.h>
#include <dlg.h>
#include <fmt/zip.h>
#include <pal.h>

#include "sld_impl.h"

typedef struct
{
    char     file_name[64];
    uint16_t method;
    uint32_t crc32;
    uint16_t parameter;
    char     data[128];
} script_call_content;
#define CONTENT(sld) ((script_call_content *)(&sld->content))

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
_validate_dsn(const char *dsn)
{
    if (9 != strlen(dsn))
    {
        return false;
    }

    for (int i = 0; i < 9; i++)
    {
        if (4 == i)
        {
            if ('-' != dsn[i])
            {
                return false;
            }
        }
        else
        {
            if (!isxdigit(dsn[i]))
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
    char msg_enterpass[40], msg_enterpass_desc[80], msg_invalidkey[40];
    pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
    pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                    sizeof(msg_enterpass_desc));
    pal_load_string(IDS_INVALIDKEY, msg_invalidkey, sizeof(msg_invalidkey));

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

        int length = DLG_INCOMPLETE;
        dlg_prompt(msg_enterpass, msg_enterpass_desc, buffer, code_len * 2,
                   precheck);
        while (DLG_INCOMPLETE == length)
        {
            length = dlg_handle();
        }

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

        int response = DLG_INCOMPLETE;
        dlg_alert(msg_enterpass, msg_invalidkey);
        while (DLG_INCOMPLETE == response)
        {
            response = dlg_handle();
        }
    }
}

static bool
_prompt_dsn(char *dsn)
{
    char msg_enterdsn[40], msg_enterdsn_desc[80];
    pal_load_string(IDS_ENTERDSN, msg_enterdsn, sizeof(msg_enterdsn));
    pal_load_string(IDS_ENTERDSN_DESC, msg_enterdsn_desc,
                    sizeof(msg_enterdsn_desc));

    int length = DLG_INCOMPLETE;
    dlg_prompt(msg_enterdsn, msg_enterdsn_desc, dsn, 9, _validate_dsn);
    while (DLG_INCOMPLETE == length)
    {
        length = dlg_handle();
    }

    return 0 != length;
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
            char dsn[10];
            if (!_prompt_dsn(dsn))
            {
                invalid = true;
                break;
            }

            uint32_t high = strtoul(dsn, NULL, 16);
            uint32_t low = strtoul(dsn + 5, NULL, 16);
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
    int status = 0;

    sld_context *script = sld_create_context(CONTENT(sld)->file_name, NULL);
    if (NULL == script)
    {
        __sld_errmsgcpy(sld, IDS_NOEXECCTX);
        return SLD_SYSERR;
    }

    __sld_accumulator = 0;

    // Run if stored as plain text
    if ((SLD_METHOD_STORE == CONTENT(sld)->method) ||
        (zip_calculate_crc(script->data, script->size) == CONTENT(sld)->crc32))
    {
        sld_run(script);
        sld_enter_context(script);
        return status;
    }

    crg_stream crs;
    union {
        uint64_t qw;
        uint8_t  bs[sizeof(uint64_t)];
    } key;

    crg_prepare(&crs, CRG_XOR, script->data, script->size, key.bs, 6);

    if (SLD_PARAMETER_XOR48_DISKID < CONTENT(sld)->parameter)
    {
        __sld_errmsgcpy(sld, IDS_UNKNOWNKEYSRC);
        return SLD_SYSERR;
    }

    if (_acquire_xor_key(&key.qw, &crs, CONTENT(sld)->crc32,
                         CONTENT(sld)->parameter, CONTENT(sld)->data))
    {
        crg_decrypt(&crs, script->data);
        sld_run(script);
        sld_enter_context(script);
    }

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
        if ((sizeof(CONTENT(out)->file_name) - 1) < length)
        {
            __sld_errmsgcpy(out, IDS_LONGNAME);
            return SLD_ARGERR;
        }
        CONTENT(out)->file_name[length] = *(cur++);
        length++;
    }
    CONTENT(out)->file_name[length] = 0;
    out->length = length;

    if (('\r' == *cur) || ('\n' == *cur))
    {
        CONTENT(out)->method = SLD_METHOD_STORE;
        CONTENT(out)->crc32 = 0;
        CONTENT(out)->parameter = 0;
    }
    else
    {
        cur += __sld_loadu(cur, &CONTENT(out)->method);
        CONTENT(out)->crc32 = strtoul(cur, (char **)&cur, 16);
        cur += __sld_loadu(cur, &CONTENT(out)->parameter);
    }

    length = 0;
    while (('\r' != *cur) && ('\n' != *cur))
    {
        if ((sizeof(CONTENT(out)->data) - 1) < length)
        {
            __sld_errmsgcpy(out, IDS_LONGCONTENT);
            return SLD_ARGERR;
        }
        CONTENT(out)->data[length] = *(cur++);
        length++;
    }
    CONTENT(out)->data[length] = 0;

    return cur - str;
}
