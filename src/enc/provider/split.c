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

static encui_textbox_data _passcode_textbox = {NULL};

static encui_field _passcode_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_ENTERPASS_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&_passcode_textbox},
    {ENCUIFT_CHECKBOX, ENCUIFF_STATIC, IDS_STOREKEY},
};

static encui_page _pages[] = {            //
    {IDS_ENTERPASS, _passcode_page_proc}, //
    {0}};

int
__enc_split_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        encui_enter(_pages, 1);

        if (ENC_XOR == enc->cipher)
        {
            enc->data.split.local_part = strtoul(enc->parameter, NULL, 16);
            return CONTINUE;
        }

        _pages[0].data = enc;
        _pages[0].length = lengthof(_passcode_fields);
        _pages[0].fields = _passcode_fields;
        _pages[0].fields[0].data = IDS_ENTERPASS_DESC;
        _passcode_textbox.buffer = enc->buffer;
        _passcode_textbox.capacity = XOR48_PASSCODE_SIZE * 2;
        _passcode_textbox.length = 0;

        if (enc_has_key_store())
        {
            _passcode_fields[3].flags |= ENCUIFF_CHECKED;
        }
        else
        {
            _pages[0].length--;
        }

        encui_set_page(0);
        return CONTINUE;
    }

    case ENCM_TRANSFORM: {
        uint32_t key_src[2];
        enc->data.split.passcode = rstrtoull(enc->buffer, 10);
        key_src[0] = enc->data.split.local_part;
        key_src[1] = enc->data.split.passcode;
        enc_decode_key(key_src, enc->key.b, ENC_KEYSM_LE32B6D);
        return 0;
    }

    case ENCM_GET_ERROR_STRING: {
        return IDS_INVALIDPASS;
    }
    }

    return -ENOSYS;
}
