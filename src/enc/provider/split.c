#include <stdlib.h>

#include <dlg.h>
#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"

enum
{
    STATE_PASSCODE_PROMPT = ENCS_PROVIDER_START,
    STATE_PASSCODE_TYPE,
};

#define XOR48_PASSCODE_SIZE 3

static int
_handle_passcode_prompt(enc_context *enc)
{
    char msg_enterpass[40], msg_enterpass_desc[80];
    pal_load_string(IDS_ENTERPASS, msg_enterpass, sizeof(msg_enterpass));
    pal_load_string(IDS_ENTERPASS_DESC, msg_enterpass_desc,
                    sizeof(msg_enterpass_desc));

    dlg_prompt(msg_enterpass, msg_enterpass_desc, enc->buffer,
               XOR48_PASSCODE_SIZE * 2, isdigstr);
    enc->state = STATE_PASSCODE_TYPE;
    return CONTINUE;
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

    enc->data.split.passcode = rstrtoull(enc->buffer, 10);
    uint32_t key_src[2] = {enc->data.split.local_part,
                           enc->data.split.passcode};
    enc->key.qw = enc_decode_key(key_src, ENC_KEYSM_LE32B6D);
    enc->state = ENCS_VERIFY;
    return CONTINUE;
}

int
__enc_split_acquire(enc_context *enc)
{
    if (ENC_XOR == enc->cipher)
    {
        enc->data.split.local_part = strtoul(enc->parameter, NULL, 16);
        enc->state = STATE_PASSCODE_PROMPT;
        return CONTINUE;
    }

    return -EINVAL;
}

int
__enc_split_handle(enc_context *enc)
{
    switch (enc->state)
    {
    case STATE_PASSCODE_PROMPT:
        return _handle_passcode_prompt(enc);
    case STATE_PASSCODE_TYPE:
        return _handle_passcode_type(enc);
    }

    return -ENOSYS;
}

enc_provider_impl __enc_split_impl = {&__enc_split_acquire,
                                      &__enc_split_handle};