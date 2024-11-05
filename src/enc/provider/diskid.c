#include <ctype.h>
#include <stdlib.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

#define XOR48_PASSCODE_SIZE 3
#define XOR48_DSN_LENGTH    9

static int
_dsn_page_proc(int msg, void *param, void *data)
{
    if (ENCUIM_CHECK != msg)
    {
        return -ENOSYS;
    }

    const char *dsn = (const char *)param;
    if (NULL == dsn)
    {
        return 1;
    }

    if (9 != strlen(dsn))
    {
        return 1;
    }

    for (int i = 0; i < 9; i++)
    {
        if (4 == i)
        {
            if ('-' != dsn[i])
            {
                return 1;
            }
        }
        else
        {
            if (!isxdigit(dsn[i]))
            {
                return 1;
            }
        }
    }

    return 0;
}

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

            encui_page page = {IDS_ENTERDSN, IDS_ENTERDSN_DESC,
                               enc->data.diskid.dsn, XOR48_DSN_LENGTH,
                               _dsn_page_proc};
            encui_prompt(&page);
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
        return IDS_INVALIDDSNPASS;
    }
    }

    return -ENOSYS;
}
