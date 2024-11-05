#include <stdlib.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

#define XOR48_PASSCODE_SIZE 3

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

        return isdigstr(passcode) ? 0 : 1;
    }

    case ENCUIM_NEXT: {
        return __enc_decrypt_content((enc_context *)data);
    }
    }

    return -ENOSYS;
}

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
        encui_page page = {IDS_ENTERPASS,       IDS_ENTERPASS_DESC,
                           enc->buffer,         XOR48_PASSCODE_SIZE * 2,
                           _passcode_page_proc, enc};
        encui_prompt(&page);
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
