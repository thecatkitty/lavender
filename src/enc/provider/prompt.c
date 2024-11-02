#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

enum
{
    STATE_PASSCODE_PROMPT = ENCS_PROVIDER_START,
    STATE_PASSCODE_TYPE,
};

static bool
_ispkey(const char *str)
{
    return enc_validate_key_format(str, ENC_KEYSM_PKEY25XOR12);
}

static int
_handle_passcode_prompt(enc_context *enc)
{
    char msg_enterpass[GFX_COLUMNS / 2], msg_enterpass_desc[GFX_COLUMNS];

    if (ENC_KEYSM_RAW == (enc->provider >> 8))
    {
        // Raw key - hexadecimal string
        pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
        pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                        sizeof(msg_enterpass_desc));
        encui_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer,
                     enc->stream.key_length * 2, isxdigstr, enc);
        enc->state = STATE_PASSCODE_TYPE;
        return CONTINUE;
    }

    if (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
    {
        // Raw key - hexadecimal string
        pal_load_string(IDS_ENTERPKEY, msg_enterpass, sizeof(msg_enterpass));
        pal_load_string(IDS_ENTERPKEY_DESC, msg_enterpass_desc,
                        sizeof(msg_enterpass_desc));
        encui_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer, 5 * 5 + 4,
                     _ispkey, enc);
        enc->state = STATE_PASSCODE_TYPE;
        return CONTINUE;
    }

    return -EINVAL;
}

static int
_handle_passcode_type(enc_context *enc)
{
    int status = encui_handle();
    if (ENCUI_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    if (0 == status)
    {
        // Aborted typing of Passcode
        return -EACCES;
    }

    enc->state = ENCS_COMPLETE;
    return CONTINUE;
}

static int
_handle_transform(enc_context *enc)
{
    if (ENC_KEYSM_RAW == (enc->provider >> 8))
    {
        if (ENC_XOR == enc->cipher)
        {
            enc->key.qw = rstrtoull(enc->buffer, 16);
            enc->state = ENCS_VERIFY;
            return CONTINUE;
        }

        if (ENC_DES == enc->cipher)
        {
            for (int i = 0; i < sizeof(uint64_t); i++)
            {
                enc->key.b[i] = xtob((const char *)enc->buffer + (2 * i));
            }

            enc->state = ENCS_VERIFY;
            return CONTINUE;
        }

        return -EINVAL;
    }

    if (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
    {
        enc->key.qw = __builtin_bswap64(
            enc_decode_key(enc->buffer, ENC_KEYSM_PKEY25XOR12));
        enc->state = ENCS_VERIFY;
        return CONTINUE;
    }

    return -EINVAL;
}

int
__enc_prompt_acquire(enc_context *enc)
{
    if (ENC_XOR == enc->cipher)
    {
        encui_enter();
        enc->stream.key_length = 6;
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    if (ENC_DES == enc->cipher)
    {
        encui_enter();
        enc->stream.key_length = sizeof(uint64_t);
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    return -EINVAL;
}

int
__enc_prompt_handle(enc_context *enc)
{
    switch (enc->state)
    {
    case STATE_PASSCODE_PROMPT:
        return _handle_passcode_prompt(enc);
    case STATE_PASSCODE_TYPE:
        return _handle_passcode_type(enc);
    case ENCS_TRANSFORM:
        return _handle_transform(enc);
    case ENCS_INVALID:
        return (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
                   ? IDS_INVALIDPKEY
                   : IDS_INVALIDPASS;
    }

    return -ENOSYS;
}

enc_provider_impl __enc_prompt_impl = {&__enc_prompt_acquire,
                                       &__enc_prompt_handle};
