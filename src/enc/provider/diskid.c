#include <ctype.h>
#include <stdlib.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

#define XOR48_PASSCODE_SIZE 3
#define XOR48_DSN_LENGTH    9

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

int
__enc_diskid_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        if (ENC_XOR == enc->cipher)
        {
            encui_enter();

            uint32_t medium_id = pal_get_medium_id(enc->parameter);
            if (0 != medium_id)
            {
                enc->data.diskid.split.local_part = medium_id;
                return CONTINUE;
            }

            char msg_enterdsn[GFX_COLUMNS / 2], msg_enterdsn_desc[GFX_COLUMNS];
            pal_load_string(IDS_ENTERDSN, msg_enterdsn, sizeof(msg_enterdsn));
            pal_load_string(IDS_ENTERDSN_DESC, msg_enterdsn_desc,
                            sizeof(msg_enterdsn_desc));

            encui_prompt(msg_enterdsn, msg_enterdsn_desc, enc->data.diskid.dsn,
                         XOR48_DSN_LENGTH, _validate_dsn, NULL);
            return CUSTOM;
        }

        return -EINVAL;
    }

    case ENCM_CUSTOM: {
        int status = encui_handle();
        if (ENCUI_INCOMPLETE == status)
        {
            return CUSTOM;
        }

        if (0 == status)
        {
            return -EACCES;
        }

        uint32_t high = strtoul(enc->data.diskid.dsn, NULL, 16);
        uint32_t low = strtoul(enc->data.diskid.dsn + 5, NULL, 16);
        enc->data.diskid.split.local_part = (high << 16) | low;
        return 0;
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
        return IDS_INVALIDDSNPASS;
    }
    }

    return -ENOSYS;
}
