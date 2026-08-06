#include <stdlib.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"

#define XOR48_PASSCODE_SIZE 3

static int
_passcode_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case SHIZM_CHECK: {
        const char *passcode = (const char *)param;
        if (NULL == passcode)
        {
            return 1;
        }

        return isdigstr(passcode) ? 0 : 1;
    }

    case SHIZM_NEXT: {
        return __enc_decrypt_content((enc_context *)data);
    }
    }

    return -ENOSYS;
}

static shiz_textbox_data _passcode_textbox = {NULL};

static shiz_field _passcode_fields[] = {
    {SHIZFT_LABEL, SHIZFF_STATIC, IDS_ENTERPASS_DESC},
    {SHIZFT_SEPARATOR, 0, 1},
    {SHIZFT_TEXTBOX, 0, (intptr_t)&_passcode_textbox},
    {SHIZFT_CHECKBOX, SHIZFF_STATIC, IDS_STOREKEY},
};

static shiz_page _pages[] = {                                      //
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
            _passcode_fields[3].flags |= SHIZFF_CHECKED;
        }
        else
        {
            _pages[0].length--;
        }

        shiz_set_page(0);
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
