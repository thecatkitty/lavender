#include <ctype.h>
#include <stdlib.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

enum
{
    STATE_DSN_GET = ENCS_PROVIDER_START,
    STATE_DSN_PROMPT,
    STATE_DSN_TYPE,
    STATE_PASSCODE_PROMPT,
};

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
    char msg_enterdsn[GFX_COLUMNS / 2], msg_enterdsn_desc[GFX_COLUMNS];
    pal_load_string(IDS_ENTERDSN, msg_enterdsn, sizeof(msg_enterdsn));
    pal_load_string(IDS_ENTERDSN_DESC, msg_enterdsn_desc,
                    sizeof(msg_enterdsn_desc));

    encui_prompt(msg_enterdsn, msg_enterdsn_desc, enc->data.diskid.dsn,
                 XOR48_DSN_LENGTH, _validate_dsn, NULL);

    enc->state = STATE_DSN_TYPE;
    return CONTINUE;
}

static int
_handle_dsn_type(enc_context *enc)
{
    int status = encui_handle();
    if (ENCUI_INCOMPLETE == status)
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
    char msg_enterpass[GFX_COLUMNS / 2], msg_enterpass_desc[GFX_COLUMNS];
    pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
    pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                    sizeof(msg_enterpass_desc));

    encui_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer,
                 XOR48_PASSCODE_SIZE * 2, isdigstr, enc);
    enc->state = ENCS_READ;
    return CONTINUE;
}

static int
_handle_transform(enc_context *enc)
{
    enc->data.split.passcode = rstrtoull(enc->buffer, 10);
    uint32_t key_src[2] = {enc->data.split.local_part,
                           enc->data.split.passcode};
    enc->key.qw = enc_decode_key(key_src, ENC_KEYSM_LE32B6D);
    enc->state = ENCS_VERIFY;
    return CONTINUE;
}

int
__enc_diskid_acquire(enc_context *enc)
{
    if (ENC_XOR == enc->cipher)
    {
        encui_enter();
        enc->state = STATE_DSN_GET;
        return CONTINUE;
    }

    return -EINVAL;
}

int
__enc_diskid_handle(enc_context *enc)
{
    switch (enc->state)
    {
    case STATE_PASSCODE_PROMPT:
        return _handle_passcode_prompt(enc);
    case STATE_DSN_GET:
        return _handle_dsn_get(enc);
    case STATE_DSN_PROMPT:
        return _handle_dsn_prompt(enc);
    case STATE_DSN_TYPE:
        return _handle_dsn_type(enc);
    case ENCS_TRANSFORM:
        return _handle_transform(enc);
    case ENCS_INVALID:
        return IDS_INVALIDDSNPASS;
    }

    return -ENOSYS;
}

enc_provider_impl __enc_diskid_impl = {&__enc_diskid_acquire,
                                       &__enc_diskid_handle};
