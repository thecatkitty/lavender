#include <stdlib.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

#define XOR48_PASSCODE_SIZE 3

int
__enc_split_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        if (ENC_XOR == enc->cipher)
        {
            encui_enter();
            enc->data.split.local_part = strtoul(enc->parameter, NULL, 16);
            return CONTINUE;
        }

        return -EINVAL;
    }

    case ENCM_ACQUIRE: {
        char msg_enterpass[GFX_COLUMNS / 2], msg_enterpass_desc[GFX_COLUMNS];
        pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
        pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                        sizeof(msg_enterpass_desc));
        encui_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer,
                     XOR48_PASSCODE_SIZE * 2, isdigstr, enc);
        return CONTINUE;
    }

    case ENCM_TRANSFORM: {
        enc->data.split.passcode = rstrtoull(enc->buffer, 10);
        uint32_t key_src[2] = {enc->data.split.local_part,
                               enc->data.split.passcode};
        enc->key.qw = enc_decode_key(key_src, ENC_KEYSM_LE32B6D);
        return 0;
    }

    case ENCM_GET_ERROR_STRING: {
        return IDS_INVALIDPASS;
    }
    }

    return -ENOSYS;
}
