#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <crg.h>
#include <dlg.h>
#include <fmt/zip.h>
#include <pal.h>

#include "sld_impl.h"

#define XOR48_KEY_SIZE      6
#define XOR48_PROMPT_BASE   16
#define XOR48_PASSCODE_SIZE 3
#define XOR48_PASSCODE_BASE 10
#define XOR48_DSN_LENGTH    9

typedef struct
{
    // External data
    char     file_name[40];
    uint16_t method;
    uint32_t crc32;
    uint16_t parameter;
    char     data[88];

    // Execution state
    int16_t      state;
    sld_context *context;
    crg_stream   crs;
    uquad        key;
    uint32_t     local_part;
    uint32_t     passcode;
    char         dsn[16];
    char         buffer[24];
} script_call_content;

#define CONTENT(sld) ((script_call_content *)(&sld->content))
static_assert(sizeof(script_call_content) <= sizeofm(sld_entry, content),
              "Script call context larger than available space");

// Size of the plaintext stored at the end of the data buffer
#define PT_SIZE(ctx)                                                           \
    (*(uint32_t *)((char *)(ctx)->data + (ctx)->size - sizeof(uint32_t)))

enum
{
    STATE_PREPARE,
    STATE_DECODE,
    STATE_ENTER,
    STATE_KEY_ACQUIRE,
    STATE_KEY_COMBINE,
    STATE_KEY_VALIDATE,
    STATE_PASSCODE_PROMPT,
    STATE_PASSCODE_TYPE,
    STATE_PASSCODE_VALIDATE,
    STATE_PASSCODE_INVALID,
    STATE_DSN_GET,
    STATE_DSN_PROMPT,
    STATE_DSN_TYPE
};

typedef bool (*prompt_predicate)(const char *);

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
_ispkey(const char *str)
{
    size_t i;
    for (i = 0; i < PKEY25XOR12_LENGTH + 4; i++)
    {
        if (5 == (i % 6))
        {
            if ('-' != str[i])
            {
                return false;
            }
        }
        else if ((0 == str[i]) ||
                 (NULL == strchr(PKEY25XOR12_ALPHABET, str[i])))
        {
            return false;
        }
    }

    return PKEY25XOR12_LENGTH + 4 == i;
}

static uint8_t
_axtob(const char *str)
{
    if (!isxdigit(str[0]) || !isxdigit(str[1]))
    {
        return 0;
    }

    uint8_t ret =
        isdigit(str[1]) ? (str[1] - '0') : (toupper(str[1]) - 'A' + 10);
    ret |= (isdigit(str[0]) ? (str[0] - '0') : (toupper(str[0]) - 'A' + 10))
           << 4;

    return ret;
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

static int
_handle_prepare(sld_entry *sld)
{
    sld_context *script = sld_create_context(
        CONTENT(sld)->file_name,
        (SLD_METHOD_STORE == CONTENT(sld)->method) ? O_RDONLY : O_RDWR);
    if (NULL == script)
    {
        __sld_errmsgcpy(sld, IDS_NOEXECCTX);
        return SLD_SYSERR;
    }

    CONTENT(sld)->context = script;
    __sld_accumulator = 0;

    // Run if stored as plain text
    if (SLD_METHOD_STORE == CONTENT(sld)->method)
    {
        CONTENT(sld)->state = STATE_ENTER;
        return CONTINUE;
    }

    if ((SLD_METHOD_XOR48 == CONTENT(sld)->method) &&
        (zip_calculate_crc(script->data, script->size) == CONTENT(sld)->crc32))
    {
        CONTENT(sld)->state = STATE_ENTER;
        return CONTINUE;
    }

    if (SLD_METHOD_DES == CONTENT(sld)->method)
    {
        uint32_t pt_size = PT_SIZE(script);
        if ((script->size > pt_size) &&
            zip_calculate_crc(script->data, pt_size) == CONTENT(sld)->crc32)
        {
            script->size = pt_size;
            CONTENT(sld)->state = STATE_ENTER;
            return CONTINUE;
        }
    }

    // Go to key acquisition if encrypted
    CONTENT(sld)->state = STATE_KEY_ACQUIRE;
    return CONTINUE;
}

static int
_handle_key_acquire_xor48(sld_entry *sld)
{
    crg_prepare(&CONTENT(sld)->crs, CRG_XOR, CONTENT(sld)->context->data,
                CONTENT(sld)->context->size, CONTENT(sld)->key.b, 6);

    if (SLD_PARAMETER_XOR48_INLINE == CONTENT(sld)->parameter)
    {
        CONTENT(sld)->key.qw = rstrtoull(CONTENT(sld)->data, 16);
        CONTENT(sld)->crs.key = CONTENT(sld)->key.b;
        CONTENT(sld)->state = STATE_KEY_VALIDATE;
        return CONTINUE;
    }

    if (SLD_PARAMETER_XOR48_DISKID == CONTENT(sld)->parameter)
    {
        CONTENT(sld)->state = STATE_DSN_GET;
        return CONTINUE;
    }

    if (SLD_PARAMETER_XOR48_PROMPT == CONTENT(sld)->parameter)
    {
        CONTENT(sld)->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    if (SLD_PARAMETER_XOR48_SPLIT == CONTENT(sld)->parameter)
    {
        CONTENT(sld)->local_part = strtoul(CONTENT(sld)->data, NULL, 16);
        CONTENT(sld)->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    __sld_errmsgcpy(sld, IDS_UNKNOWNKEYSRC);
    return SLD_SYSERR;
}

static int
_handle_key_acquire_des(sld_entry *sld)
{
    if (SLD_PARAMETER_DES_INLINE == CONTENT(sld)->parameter)
    {
        for (int i = 0; i < sizeof(uint64_t); i++)
        {
            CONTENT(sld)->key.b[i] = _axtob(CONTENT(sld)->data + (2 * i));
        }

        crg_prepare(&CONTENT(sld)->crs, CRG_DES, CONTENT(sld)->context->data,
                    CONTENT(sld)->context->size, CONTENT(sld)->key.b,
                    sizeof(uint64_t));
        CONTENT(sld)->state = STATE_KEY_VALIDATE;
        return CONTINUE;
    }

    if ((SLD_PARAMETER_DES_PROMPT == CONTENT(sld)->parameter) ||
        (SLD_PARAMETER_DES_PKEY == CONTENT(sld)->parameter))
    {
        CONTENT(sld)->crs.key_length = sizeof(uint64_t);
        CONTENT(sld)->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    __sld_errmsgcpy(sld, IDS_UNKNOWNKEYSRC);
    return SLD_SYSERR;
}

static int
_handle_key_acquire(sld_entry *sld)
{
    if (SLD_METHOD_XOR48 == CONTENT(sld)->method)
    {
        return _handle_key_acquire_xor48(sld);
    }

    if (SLD_METHOD_DES == CONTENT(sld)->method)
    {
        return _handle_key_acquire_des(sld);
    }

    __sld_errmsgcpy(sld, IDS_UNKNOWNKEYSRC);
    return SLD_SYSERR;
}

static int
_handle_dsn_get(sld_entry *sld)
{
    uint32_t medium_id = pal_get_medium_id(CONTENT(sld)->data);
    if (0 != medium_id)
    {
        CONTENT(sld)->local_part = medium_id;
        CONTENT(sld)->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    CONTENT(sld)->state = STATE_DSN_PROMPT;
    return CONTINUE;
}

static int
_handle_dsn_prompt(sld_entry *sld)
{
    char msg_enterdsn[40], msg_enterdsn_desc[80];
    pal_load_string(IDS_ENTERDSN, msg_enterdsn, sizeof(msg_enterdsn));
    pal_load_string(IDS_ENTERDSN_DESC, msg_enterdsn_desc,
                    sizeof(msg_enterdsn_desc));

    dlg_prompt(msg_enterdsn, msg_enterdsn_desc, CONTENT(sld)->dsn,
               XOR48_DSN_LENGTH, _validate_dsn);

    CONTENT(sld)->state = STATE_DSN_TYPE;
    return CONTINUE;
}

static int
_handle_dsn_type(sld_entry *sld)
{
    int status = dlg_handle();
    if (DLG_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    if (0 == status)
    {
        // Aborted typing of Disk Serial Number
        sld_close_context(CONTENT(sld)->context);
        CONTENT(sld)->context = 0;
        __sld_accumulator = UINT16_MAX;
        return 0;
    }

    uint32_t high = strtoul(CONTENT(sld)->dsn, NULL, 16);
    uint32_t low = strtoul(CONTENT(sld)->dsn + 5, NULL, 16);
    CONTENT(sld)->local_part = (high << 16) | low;
    CONTENT(sld)->state = STATE_PASSCODE_PROMPT;
    return CONTINUE;
}

static int
_handle_passcode_prompt(sld_entry *sld)
{
    char msg_enterpass[40], msg_enterpass_desc[80];
    pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
    pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                    sizeof(msg_enterpass_desc));

    prompt_predicate precheck = NULL;
    int              code_len = sizeofm(script_call_content, buffer) - 1;

    if (SLD_KEYSOURCE_PROMPT == CONTENT(sld)->parameter)
    {
        precheck = _isxdigitstr;
        code_len = CONTENT(sld)->crs.key_length * 2;
    }
    else if (SLD_METHOD_XOR48 == CONTENT(sld)->method)
    {
        precheck = _isdigitstr;
        code_len = XOR48_PASSCODE_SIZE * 2;
    }
    else if ((SLD_METHOD_DES == CONTENT(sld)->method) &&
             (SLD_PARAMETER_DES_PKEY == CONTENT(sld)->parameter))
    {
        precheck = _ispkey;
        code_len = PKEY25XOR12_LENGTH + 4;
    }

    dlg_prompt(msg_enterpass, msg_enterpass_desc, CONTENT(sld)->buffer,
               code_len, precheck);
    CONTENT(sld)->state = STATE_PASSCODE_TYPE;
    return CONTINUE;
}

static int
_handle_passcode_type(sld_entry *sld)
{
    int status = dlg_handle();
    if (DLG_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    if (0 == status)
    {
        // Aborted typing of Passcode
        sld_close_context(CONTENT(sld)->context);
        CONTENT(sld)->context = 0;
        __sld_accumulator = UINT16_MAX;
        return 0;
    }

    int base = 10;
    if (SLD_KEYSOURCE_PROMPT == CONTENT(sld)->parameter)
    {
        base = 16;
    }

    if ((SLD_METHOD_DES == CONTENT(sld)->method) &&
        (SLD_PARAMETER_DES_PKEY == CONTENT(sld)->parameter))
    {
        char        pkey[PKEY25XOR12_LENGTH];
        const char *src = CONTENT(sld)->buffer;
        char       *dst = pkey;
        while (dst < pkey + PKEY25XOR12_LENGTH)
        {
            if ('-' != *src)
            {
                *dst = *src;
                dst++;
            }
            src++;
        }
        CONTENT(sld)->key.qw =
            __builtin_bswap64(crg_decode_key(pkey, CRG_KEYSM_PKEY25XOR12));
    }
    else
    {
        CONTENT(sld)->passcode = rstrtoull(CONTENT(sld)->buffer, base);
    }
    CONTENT(sld)->state = STATE_PASSCODE_VALIDATE;
    return CONTINUE;
}

static void
_handle_passcode_validate_xor48(sld_entry *sld)
{
    if (SLD_PARAMETER_XOR48_PROMPT == CONTENT(sld)->parameter)
    {
        for (int i = 0; i < 6; i++)
        {
            CONTENT(sld)->key.b[i] = _axtob(CONTENT(sld)->buffer + i * 2);
        }
    }
    else
    {
        // 48-bit split key
        uint32_t key_src[2] = {CONTENT(sld)->local_part,
                               CONTENT(sld)->passcode};
        CONTENT(sld)->key.qw = crg_decode_key(key_src, CRG_KEYSM_LE32B6D);
    }

    CONTENT(sld)->crs.key = CONTENT(sld)->key.b;
}

static void
_handle_passcode_validate_des(sld_entry *sld)
{
    if (SLD_PARAMETER_DES_PKEY != CONTENT(sld)->parameter)
    {
        for (int i = 0; i < sizeof(uint64_t); i++)
        {
            CONTENT(sld)->key.b[i] = _axtob(CONTENT(sld)->buffer + (2 * i));
        }
    }

    crg_prepare(&CONTENT(sld)->crs, CRG_DES, CONTENT(sld)->context->data,
                CONTENT(sld)->context->size, CONTENT(sld)->key.b,
                sizeof(uint64_t));
}

static int
_handle_passcode_validate(sld_entry *sld)
{
    if (SLD_METHOD_XOR48 == CONTENT(sld)->method)
    {
        _handle_passcode_validate_xor48(sld);
    }

    if (SLD_METHOD_DES == CONTENT(sld)->method)
    {
        _handle_passcode_validate_des(sld);
    }

    CONTENT(sld)->state =
        (crg_validate(&CONTENT(sld)->crs, CONTENT(sld)->crc32))
            ? STATE_KEY_VALIDATE
            : STATE_PASSCODE_INVALID;

    if (STATE_PASSCODE_INVALID == CONTENT(sld)->state)
    {
        if (SLD_METHOD_DES == CONTENT(sld)->method)
        {
            crg_free(&CONTENT(sld)->crs);
        }

        char msg_enterpass[40], msg_invalidkey[40];
        pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
        pal_load_string(IDS_INVALIDKEY, msg_invalidkey, sizeof(msg_invalidkey));

        dlg_alert(msg_enterpass, msg_invalidkey);
    }

    return CONTINUE;
}

static int
_handle_passcode_invalid(sld_entry *sld)
{
    int status = dlg_handle();
    if (DLG_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    CONTENT(sld)->state = STATE_PASSCODE_PROMPT;
    return CONTINUE;
}

static int
_handle_key_validate(sld_entry *sld)
{
    if (!crg_validate(&CONTENT(sld)->crs, CONTENT(sld)->crc32))
    {
        __sld_accumulator = UINT16_MAX;
        return 0;
    }

    __sld_accumulator = 0;
    CONTENT(sld)->state = STATE_DECODE;
    return CONTINUE;
}

static int
_handle_decode(sld_entry *sld)
{
    crg_decrypt(&CONTENT(sld)->crs, (uint8_t *)CONTENT(sld)->context->data);
    crg_free(&CONTENT(sld)->crs);
    if (SLD_METHOD_DES == CONTENT(sld)->method)
    {
        PT_SIZE(CONTENT(sld)->context) = CONTENT(sld)->crs.data_length;
        CONTENT(sld)->context->size = CONTENT(sld)->crs.data_length;
    }

    CONTENT(sld)->state = STATE_ENTER;
    return CONTINUE;
}

static int
_handle_enter(sld_entry *sld)
{
    sld_run(CONTENT(sld)->context);
    sld_enter_context(CONTENT(sld)->context);
    CONTENT(sld)->state = STATE_PREPARE;
    return 0;
}

int
__sld_handle_script_call(sld_entry *sld)
{
    switch (CONTENT(sld)->state)
    {
    case STATE_PREPARE:
        return _handle_prepare(sld);
    case STATE_DECODE:
        return _handle_decode(sld);
    case STATE_ENTER:
        return _handle_enter(sld);
    case STATE_KEY_ACQUIRE:
        return _handle_key_acquire(sld);
    case STATE_KEY_VALIDATE:
        return _handle_key_validate(sld);
    case STATE_PASSCODE_PROMPT:
        return _handle_passcode_prompt(sld);
    case STATE_PASSCODE_TYPE:
        return _handle_passcode_type(sld);
    case STATE_PASSCODE_VALIDATE:
        return _handle_passcode_validate(sld);
    case STATE_PASSCODE_INVALID:
        return _handle_passcode_invalid(sld);
    case STATE_DSN_GET:
        return _handle_dsn_get(sld);
    case STATE_DSN_PROMPT:
        return _handle_dsn_prompt(sld);
    case STATE_DSN_TYPE:
        return _handle_dsn_type(sld);
    }

    __sld_errmsgcpy(sld, IDS_UNSUPPORTED);
    return SLD_SYSERR;
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

    CONTENT(out)->state = STATE_PREPARE;
    return cur - str;
}
