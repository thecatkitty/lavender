#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

static int
_passcode_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_CHECK: {
        const char *passcode = (const char *)param;
        if (NULL == passcode)
        {
            return 1;
        }

        const enc_context *enc = (const enc_context *)data;
        if (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
        {
            return enc_validate_key_format(passcode, ENC_KEYSM_PKEY25XOR12) ? 0
                                                                            : 1;
        }

        return isxdigstr(passcode) ? 0 : 1;
    }

    case ENCUIM_NEXT: {
        return __enc_decrypt_content((enc_context *)data);
    }
    }

    return -ENOSYS;
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
        encui_page page = {IDS_ENTERPASS,       IDS_ENTERPASS_DESC,
                           enc->buffer,         enc->stream.key_length * 2,
                           _passcode_page_proc, enc};
        if (ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8))
        {
            page.title = IDS_ENTERPKEY;
            page.message = IDS_ENTERPKEY_DESC;
            page.capacity = 5 * 5 + 4;
        }

        encui_prompt(&page);
        return CONTINUE;
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
