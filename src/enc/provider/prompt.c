#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

static bool
_ispkey(const char *str)
{
    return enc_validate_key_format(str, ENC_KEYSM_PKEY25XOR12);
}

int
__enc_prompt_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        if (ENC_XOR == enc->cipher)
        {
            encui_enter();
            enc->stream.key_length = 6;
            return CONTINUE;
        }

        if (ENC_DES == enc->cipher)
        {
            encui_enter();
            enc->stream.key_length = sizeof(uint64_t);
            return CONTINUE;
        }

        return -EINVAL;
    }

    case ENCM_ACQUIRE: {
        char msg_enterpass[GFX_COLUMNS / 2], msg_enterpass_desc[GFX_COLUMNS];

        if (ENC_KEYSM_RAW == (enc->provider >> 8))
        {
            // Raw key - hexadecimal string
            pal_load_string(IDS_ENTERPASS, msg_enterpass,
                            sizeof(msg_enterpass));
            pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                            sizeof(msg_enterpass_desc));
            encui_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer,
                         enc->stream.key_length * 2, isxdigstr, enc);
            return CONTINUE;
        }

        if (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
        {
            // Raw key - hexadecimal string
            pal_load_string(IDS_ENTERPKEY, msg_enterpass,
                            sizeof(msg_enterpass));
            pal_load_string(IDS_ENTERPKEY_DESC, msg_enterpass_desc,
                            sizeof(msg_enterpass_desc));
            encui_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer,
                         5 * 5 + 4, _ispkey, enc);
            return CONTINUE;
        }

        return -EINVAL;
    }

    case ENCM_TRANSFORM: {
        if (ENC_KEYSM_RAW == (enc->provider >> 8))
        {
            if (ENC_XOR == enc->cipher)
            {
                enc->key.qw = rstrtoull(enc->buffer, 16);
                return 0;
            }

            if (ENC_DES == enc->cipher)
            {
                for (int i = 0; i < sizeof(uint64_t); i++)
                {
                    enc->key.b[i] = xtob((const char *)enc->buffer + (2 * i));
                }

                return 0;
            }

            return -EINVAL;
        }

        if (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
        {
            enc->key.qw = __builtin_bswap64(
                enc_decode_key(enc->buffer, ENC_KEYSM_PKEY25XOR12));
            return 0;
        }

        return -EINVAL;
    }

    case ENCM_GET_ERROR_STRING: {
        return (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
                   ? IDS_INVALIDPKEY
                   : IDS_INVALIDPASS;
    }
    }

    return -ENOSYS;
}
