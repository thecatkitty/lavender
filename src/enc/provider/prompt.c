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
        const char        *passcode = (const char *)param;
        const enc_context *enc = (const enc_context *)data;

        if (NULL == passcode)
        {
            return 1;
        }

        if ((ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8)) ||
            (ENC_KEYSM_PKEY25XOR2B == (enc->provider >> 8)))
        {
            return enc_validate_key_format(passcode, enc->provider >> 8) ? 0
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

static encui_page _pages[] = {
    {IDS_ENTERPASS, IDS_ENTERPASS_DESC, NULL, 0, _passcode_page_proc}, {0}};

int
__enc_prompt_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        if (ENC_XOR == enc->cipher)
        {
            enc->stream.key_length = 6;
        }

        if (ENC_DES == enc->cipher)
        {
            enc->stream.key_length = sizeof(uint64_t);
        }

        if (ENC_TDES == enc->cipher)
        {
            enc->stream.key_length = 2 * sizeof(uint64_t);
        }

        _pages[0].buffer = enc->buffer;
        _pages[0].data = enc;
        _pages[0].length = 0;
        _pages[0].title = IDS_ENTERPASS;
        _pages[0].message = IDS_ENTERPASS_DESC;
        _pages[0].capacity = enc->stream.key_length * 2;

        if ((ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8)) ||
            (ENC_KEYSM_PKEY25XOR2B == (enc->provider >> 8)))
        {
            _pages[0].title = IDS_ENTERPKEY;
            _pages[0].message = IDS_ENTERPKEY_DESC;
            _pages[0].capacity = 5 * 5 + 4;
        }

        encui_enter(_pages, 1);
        encui_set_page(0);
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

            if ((ENC_DES == enc->cipher) || (ENC_TDES == enc->cipher))
            {
                int i, size = sizeof(uint64_t) *
                              ((ENC_TDES == enc->cipher) ? 2 : 1);
                for (i = 0; i < size; i++)
                {
                    enc->key.b[i] = xtob((const char *)enc->buffer + (2 * i));
                }

                return 0;
            }

            return -EINVAL;
        }

        if ((ENC_KEYSM_PKEY25XOR12 == (enc->provider >> 8)) ||
            (ENC_KEYSM_PKEY25XOR2B == (enc->provider >> 8)))
        {
            enc_decode_key(enc->buffer, enc->key.b, enc->provider >> 8);
            (&enc->key.qw)[0] = __builtin_bswap64((&enc->key.qw)[0]);
            (&enc->key.qw)[1] = __builtin_bswap64((&enc->key.qw)[1]);

            return 0;
        }

        return -EINVAL;
    }

    case ENCM_GET_ERROR_STRING: {
        return (ENC_KEYSM_RAW == (enc->provider >> 8)) ? IDS_INVALIDPASS
                                                       : IDS_INVALIDPKEY;
    }
    }

    return -ENOSYS;
}
