#include <assert.h>
#include <ctype.h>
#include <stdlib.h>

#include <enc.h>

#include "sld_impl.h"

typedef struct
{
    // External data
    char     file_name[40];
    uint16_t method;
    uint32_t crc32;
    uint16_t parameter;
    char     data[52];

    // Execution state
    int16_t      state;
    sld_context *context;
#if defined(CONFIG_ENCRYPTED_CONTENT)
    enc_context enc;
#endif // CONFIG_ENCRYPTED_CONTENT
} script_call_content;

#define CONTENT(sld) ((script_call_content *)(&sld->content))
static_assert(sizeof(script_call_content) <= sizeofm(sld_entry, content),
              "Script call context larger than available space");

enum
{
    STATE_PREPARE,
    STATE_ENTER,
#if defined(CONFIG_ENCRYPTED_CONTENT)
    STATE_DECODE,
#endif // CONFIG_ENCRYPTED_CONTENT
};

static int
_handle_prepare(sld_entry *sld)
{
#if defined(CONFIG_ENCRYPTED_CONTENT)
    enc_cipher cipher = 0;
    int        provider = 0;
    void      *parameter = NULL;
#endif

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

#if defined(CONFIG_ENCRYPTED_CONTENT)
    switch (CONTENT(sld)->parameter)
    {
    case SLD_PARAMETER_XOR48_INLINE: {
        provider = ENC_PROVIDER_CALLER;
        parameter = CONTENT(sld)->data;
        break;
    }

    case SLD_PARAMETER_XOR48_PROMPT: {
        provider = ENC_PROVIDER_PROMPT;
        break;
    }
    }

    if (SLD_METHOD_XOR48 == CONTENT(sld)->method)
    {
        cipher = ENC_XOR;

        switch (CONTENT(sld)->parameter)
        {
        case SLD_PARAMETER_XOR48_SPLIT: {
            provider = ENC_PROVIDER_SPLIT;
            parameter = CONTENT(sld)->data;
            break;
        }

        case SLD_PARAMETER_XOR48_DISKID: {
            provider = ENC_PROVIDER_DISKID;
            parameter = CONTENT(sld)->data;
            break;
        }
        }
    }

    if (SLD_METHOD_DES == CONTENT(sld)->method)
    {
        cipher = ENC_DES;

        switch (CONTENT(sld)->parameter)
        {
        case SLD_PARAMETER_DES_PKEY: {
            provider = ENC_KEYSRC(ENC_PROVIDER_PROMPT, ENC_KEYSM_PKEY25XOR12);
            break;
        }
        }
    }

    if (0 == enc_access_content(&CONTENT(sld)->enc, cipher, provider, parameter,
                                (uint8_t *)CONTENT(sld)->context->data,
                                CONTENT(sld)->context->size,
                                CONTENT(sld)->crc32))
    {
        // Already decrypted
        CONTENT(sld)->state = STATE_ENTER;
        CONTENT(sld)->context->size = CONTENT(sld)->enc.size;
        return CONTINUE;
    }

    // Go to decryption if applicable
    CONTENT(sld)->state = STATE_DECODE;
    return CONTINUE;
#else
    __sld_errmsgcpy(sld, IDS_UNSUPPORTED);
    return SLD_SYSERR;
#endif // CONFIG_ENCRYPTED_CONTENT
}

#if defined(CONFIG_ENCRYPTED_CONTENT)
static int
_handle_decode(sld_entry *sld)
{
    int status = enc_handle(&CONTENT(sld)->enc);
    if (0 == status)
    {
        CONTENT(sld)->context->size = CONTENT(sld)->enc.size;
        CONTENT(sld)->state = STATE_ENTER;
    }

    if (-EACCES == status)
    {
        sld_close_context(CONTENT(sld)->context);
        CONTENT(sld)->context = 0;
        __sld_accumulator = UINT16_MAX;
        return 0;
    }

    if (-EINVAL == status)
    {
        __sld_errmsgcpy(sld, IDS_UNKNOWNKEYSRC);
    }

    if (0 > status)
    {
        // errmsg
        return status;
    }

    return CONTINUE;
}
#endif // CONFIG_ENCRYPTED_CONTENT

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
    case STATE_ENTER:
        return _handle_enter(sld);
#if defined(CONFIG_ENCRYPTED_CONTENT)
    case STATE_DECODE:
        return _handle_decode(sld);
#endif // CONFIG_ENCRYPTED_CONTENT
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
