#include <dlg.h>
#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"

enum
{
    STATE_PASSCODE_PROMPT = ENCS_PROVIDER_START,
    STATE_PASSCODE_TYPE,
    STATE_ALERT,
};

static bool
_ispkey(const char *str)
{
    return enc_validate_key_format(str, ENC_KEYSM_PKEY25XOR12);
}

static int
_handle_passcode_prompt(enc_context *enc)
{
    char msg_enterpass[40], msg_enterpass_desc[80];
    pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
    pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                    sizeof(msg_enterpass_desc));

    if (ENC_KEYSM_RAW == (enc->provider >> 8))
    {
        // Raw key - hexadecimal string
        dlg_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer,
                   enc->stream.key_length * 2, isxdigstr);
        enc->state = STATE_PASSCODE_TYPE;
        return CONTINUE;
    }

    if (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
    {
        dlg_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer, 5 * 5 + 4,
                   _ispkey);
        enc->state = STATE_PASSCODE_TYPE;
        return CONTINUE;
    }

    return -EINVAL;
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

static int
_handle_invalid(enc_context *enc)
{
    char msg_enterpass[40], msg_invalidkey[40];
    pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
    pal_load_string(IDS_INVALIDKEY, msg_invalidkey, sizeof(msg_invalidkey));

    dlg_alert(msg_enterpass, msg_invalidkey);
    enc->state = STATE_ALERT;
    return CONTINUE;
}

static int
_handle_alert(enc_context *enc)
{
    int status = dlg_handle();
    if (DLG_INCOMPLETE == status)
    {
        return CONTINUE;
    }

    enc->state = ENCS_ACQUIRE;
    return CONTINUE;
}

int
__enc_prompt_acquire(enc_context *enc)
{
    if (ENC_XOR == enc->cipher)
    {
        enc->stream.key_length = 6;
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    if (ENC_DES == enc->cipher)
    {
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
    case ENCS_INVALID:
        return _handle_invalid(enc);
    case STATE_ALERT:
        return _handle_alert(enc);
    }

    return -ENOSYS;
}

enc_provider_impl __enc_prompt_impl = {&__enc_prompt_acquire,
                                       &__enc_prompt_handle};
