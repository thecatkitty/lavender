#include <ctype.h>
#include <stdlib.h>

#include <dlg.h>
#include <enc.h>
#include <fmt/zip.h>
#include <pal.h>

#include "../resource.h"

typedef bool (*prompt_predicate)(const char *);

enum
{
    STATE_DECODE,
    STATE_KEY_ACQUIRE,
    STATE_KEY_COMBINE,
    STATE_KEY_VERIFY,
    STATE_PASSCODE_PROMPT,
    STATE_PASSCODE_TYPE,
    STATE_PASSCODE_VERIFY,
    STATE_PASSCODE_INVALID,
    STATE_DSN_GET,
    STATE_DSN_PROMPT,
    STATE_DSN_TYPE,
    STATE_COMPLETE,
};

#define CONTINUE 1

#define XOR48_PASSCODE_SIZE 3
#define XOR48_DSN_LENGTH    9

// Size of the plaintext stored at the end of the data buffer
#define PT_SIZE(data, size)                                                    \
    (*(uint32_t *)((char *)(data) + (size) - sizeof(uint32_t)))

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
    return enc_validate_key_format(str, ENC_KEYSM_PKEY25XOR12);
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
_handle_key_acquire_xor48(enc_context *enc)
{
    enc_prepare(&enc->stream, ENC_XOR, enc->content, enc->size, enc->key.b, 6);

    if (ENC_PROVIDER_CALLER == (enc->provider & 0xFF))
    {
        enc->key.qw = rstrtoull(enc->parameter, 16);
        enc->stream.key = enc->key.b;
        enc->state = STATE_KEY_VERIFY;
        return CONTINUE;
    }

    if (ENC_PROVIDER_DISKID == (enc->provider & 0xFF))
    {
        enc->state = STATE_DSN_GET;
        return CONTINUE;
    }

    if (ENC_PROVIDER_PROMPT == (enc->provider & 0xFF))
    {
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    if (ENC_PROVIDER_SPLIT == (enc->provider & 0xFF))
    {
        enc->data.split.local_part = strtoul(enc->parameter, NULL, 16);
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    return -EINVAL;
}

static int
_handle_key_acquire_des(enc_context *enc)
{
    if (ENC_PROVIDER_CALLER == (enc->provider & 0xFF))
    {
        for (int i = 0; i < sizeof(uint64_t); i++)
        {
            enc->key.b[i] = _axtob((const char *)enc->parameter + (2 * i));
        }

        enc_prepare(&enc->stream, ENC_DES, enc->content, enc->size, enc->key.b,
                    sizeof(uint64_t));
        enc->state = STATE_KEY_VERIFY;
        return CONTINUE;
    }

    if (ENC_PROVIDER_PROMPT == (enc->provider & 0xFF))
    {
        enc->stream.key_length = sizeof(uint64_t);
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    return -EINVAL;
}

static int
_handle_key_acquire(enc_context *enc)
{
    if (ENC_XOR == enc->cipher)
    {
        return _handle_key_acquire_xor48(enc);
    }

    if (ENC_DES == enc->cipher)
    {
        return _handle_key_acquire_des(enc);
    }

    return -EINVAL;
}

static int
_handle_dsn_get(enc_context *enc)
{
    uint32_t medium_id = pal_get_medium_id(enc->parameter);
    if (0 != medium_id)
    {
        enc->data.diskid.split.local_part = medium_id;
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    enc->state = STATE_DSN_PROMPT;
    return CONTINUE;
}

static int
_handle_dsn_prompt(enc_context *enc)
{
    char msg_enterdsn[40], msg_enterdsn_desc[80];
    pal_load_string(IDS_ENTERDSN, msg_enterdsn, sizeof(msg_enterdsn));
    pal_load_string(IDS_ENTERDSN_DESC, msg_enterdsn_desc,
                    sizeof(msg_enterdsn_desc));

    dlg_prompt(msg_enterdsn, msg_enterdsn_desc, enc->data.diskid.dsn,
               XOR48_DSN_LENGTH, _validate_dsn);

    enc->state = STATE_DSN_TYPE;
    return CONTINUE;
}

static int
_handle_dsn_type(enc_context *enc)
{
    int status = dlg_handle();
    if (DLG_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    if (0 == status)
    {
        return -EACCES;
    }

    uint32_t high = strtoul(enc->data.diskid.dsn, NULL, 16);
    uint32_t low = strtoul(enc->data.diskid.dsn + 5, NULL, 16);
    enc->data.diskid.split.local_part = (high << 16) | low;
    enc->state = STATE_PASSCODE_PROMPT;
    return CONTINUE;
}

static int
_handle_passcode_prompt(enc_context *enc)
{
    char msg_enterpass[40], msg_enterpass_desc[80];
    pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
    pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                    sizeof(msg_enterpass_desc));

    prompt_predicate precheck = NULL;
    int              code_len = sizeof(enc->buffer) - 1;

    if (ENC_KEYSRC(ENC_PROVIDER_PROMPT, ENC_KEYSM_RAW) == enc->provider)
    {
        // Raw key - hexadecimal string
        precheck = _isxdigitstr;
        code_len = enc->stream.key_length * 2;
    }
    else if (ENC_XOR == enc->cipher)
    {
        // XOR48 split or DSN - decimal passcode
        precheck = _isdigitstr;
        code_len = XOR48_PASSCODE_SIZE * 2;
    }
    else if (ENC_KEYSRC(ENC_PROVIDER_PROMPT, ENC_KEYSM_PKEY25XOR12) ==
             enc->provider)
    {
        precheck = _ispkey;
        code_len = 5 * 5 + 4;
    }

    dlg_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer, code_len,
               precheck);
    enc->state = STATE_PASSCODE_TYPE;
    return CONTINUE;
}

static int
_handle_passcode_type(enc_context *enc)
{
    int status = dlg_handle();
    if (DLG_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    if (0 == status)
    {
        // Aborted typing of Passcode
        return -EACCES;
    }

    int base = 10;
    if (ENC_KEYSRC(ENC_PROVIDER_PROMPT, ENC_KEYSM_RAW) == enc->provider)
    {
        base = 16;
    }

    if (ENC_KEYSRC(ENC_PROVIDER_PROMPT, ENC_KEYSM_PKEY25XOR12) == enc->provider)
    {
        enc->key.qw = __builtin_bswap64(
            enc_decode_key(enc->buffer, ENC_KEYSM_PKEY25XOR12));
    }
    else
    {
        enc->data.split.passcode = rstrtoull(enc->buffer, base);
    }
    enc->state = STATE_PASSCODE_VERIFY;
    return CONTINUE;
}

static void
_handle_passcode_verify_xor48(enc_context *enc)
{
    if (ENC_KEYSRC(ENC_PROVIDER_PROMPT, ENC_KEYSM_RAW) == enc->provider)
    {
        for (int i = 0; i < 6; i++)
        {
            enc->key.b[i] = _axtob(enc->buffer + i * 2);
        }
    }
    else
    {
        // 48-bit split key
        uint32_t key_src[2] = {enc->data.split.local_part,
                               enc->data.split.passcode};
        enc->key.qw = enc_decode_key(key_src, ENC_KEYSM_LE32B6D);
    }

    enc->stream.key = enc->key.b;
}

static void
_handle_passcode_verify_des(enc_context *enc)
{
    if (ENC_KEYSRC(ENC_PROVIDER_PROMPT, ENC_KEYSM_RAW) == enc->provider)
    {
        for (int i = 0; i < sizeof(uint64_t); i++)
        {
            enc->key.b[i] = _axtob(enc->buffer + (2 * i));
        }
    }

    enc_prepare(&enc->stream, ENC_DES, enc->content, enc->size, enc->key.b,
                sizeof(uint64_t));
}

static int
_handle_passcode_verify(enc_context *enc)
{
    if (ENC_XOR == enc->cipher)
    {
        _handle_passcode_verify_xor48(enc);
    }

    if (ENC_DES == enc->cipher)
    {
        _handle_passcode_verify_des(enc);
    }

    enc->state = (enc_verify(&enc->stream, enc->crc32))
                     ? STATE_KEY_VERIFY
                     : STATE_PASSCODE_INVALID;

    if (STATE_PASSCODE_INVALID == enc->state)
    {
        if (ENC_DES == enc->cipher)
        {
            enc_free(&enc->stream);
        }

        char msg_enterpass[40], msg_invalidkey[40];
        pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
        pal_load_string(IDS_INVALIDKEY, msg_invalidkey, sizeof(msg_invalidkey));

        dlg_alert(msg_enterpass, msg_invalidkey);
    }

    return CONTINUE;
}

static int
_handle_passcode_invalid(enc_context *enc)
{
    int status = dlg_handle();
    if (DLG_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    enc->state = STATE_PASSCODE_PROMPT;
    return CONTINUE;
}

static int
_handle_key_verify(enc_context *enc)
{
    if (!enc_verify(&enc->stream, enc->crc32))
    {
        return -EACCES;
    }

    enc->state = STATE_DECODE;
    return CONTINUE;
}

static int
_handle_decode(enc_context *enc)
{
    enc_decrypt(&enc->stream, (uint8_t *)enc->content);
    enc_free(&enc->stream);
    if (ENC_DES == enc->cipher)
    {
        PT_SIZE(enc->content, enc->size) = enc->stream.data_length;
        enc->size = enc->stream.data_length;
    }

    enc->state = STATE_COMPLETE;
    return CONTINUE;
}

int
enc_handle(enc_context *enc)
{
    switch (enc->state)
    {
    case STATE_DECODE:
        return _handle_decode(enc);
    case STATE_KEY_ACQUIRE:
        return _handle_key_acquire(enc);
    case STATE_KEY_VERIFY:
        return _handle_key_verify(enc);
    case STATE_PASSCODE_PROMPT:
        return _handle_passcode_prompt(enc);
    case STATE_PASSCODE_TYPE:
        return _handle_passcode_type(enc);
    case STATE_PASSCODE_VERIFY:
        return _handle_passcode_verify(enc);
    case STATE_PASSCODE_INVALID:
        return _handle_passcode_invalid(enc);
    case STATE_DSN_GET:
        return _handle_dsn_get(enc);
    case STATE_DSN_PROMPT:
        return _handle_dsn_prompt(enc);
    case STATE_DSN_TYPE:
        return _handle_dsn_type(enc);
    case STATE_COMPLETE:
        return 0;
    }

    return -ENOSYS;
}

int
enc_access_content(enc_context *enc,
                   enc_cipher   cipher,
                   int          provider,
                   const void  *parameter,
                   uint8_t     *content,
                   size_t       length,
                   uint32_t     crc32)
{
    memset(enc, 0, sizeof(*enc));
    enc->cipher = cipher;
    enc->provider = provider;
    enc->parameter = parameter;
    enc->content = content;
    enc->size = length;
    enc->crc32 = crc32;

    if ((ENC_XOR == cipher) && (zip_calculate_crc(content, length) == crc32))
    {
        enc->state = STATE_COMPLETE;
        return 0;
    }

    if (ENC_DES == cipher)
    {
        // Size of the plaintext stored at the end of the content buffer
        uint32_t pt_size = PT_SIZE(content, length);
        if ((length > pt_size) && zip_calculate_crc(content, pt_size) == crc32)
        {
            enc->size = pt_size;
            enc->state = STATE_COMPLETE;
            return 0;
        }
    }

    enc->state = STATE_KEY_ACQUIRE;
    return CONTINUE;
}
