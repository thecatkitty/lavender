#include <ctype.h>
#include <stdlib.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"

#define XOR48_PASSCODE_SIZE 3
#define XOR48_DSN_LENGTH    9

static int
_dsn_page_proc(int msg, void *param, void *data)
{
    const char *dsn = (const char *)param;
    int         i;

    if (SHIZM_NEXT == msg)
    {
        enc_context *enc = (enc_context *)data;
        uint32_t     high = strtoul(enc->data.diskid.dsn, NULL, 16);
        uint32_t     low = strtoul(enc->data.diskid.dsn + 5, NULL, 16);
        enc->data.diskid.split.local_part = (high << 16) | low;
        return 0;
    }

    if (SHIZM_CHECK != msg)
    {
        return -ENOSYS;
    }

    if (NULL == dsn)
    {
        return 1;
    }

    if (9 != strlen(dsn))
    {
        return 1;
    }

    for (i = 0; i < 9; i++)
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

static shiz_textbox_data _dsn_textbox = {NULL};

static shiz_field _dsn_fields[] = {
    {SHIZFT_LABEL, SHIZFF_STATIC, IDS_ENTERDSN_DESC},
    {SHIZFT_SEPARATOR, 0, 1},
    {SHIZFT_TEXTBOX, 0, (intptr_t)&_dsn_textbox},
};

static shiz_textbox_data _passcode_textbox = {NULL};

static shiz_field _passcode_fields[] = {
    {SHIZFT_LABEL, SHIZFF_STATIC, IDS_ENTERPASS_DESC},
    {SHIZFT_SEPARATOR, 0, 1},
    {SHIZFT_TEXTBOX, 0, (intptr_t)&_passcode_textbox},
    {SHIZFT_CHECKBOX, SHIZFF_STATIC, IDS_STOREKEY},
};

static shiz_page _pages[] = {                                      //
                             {IDS_ENTERDSN, _dsn_page_proc},       //
                             {IDS_ENTERPASS, _passcode_page_proc}, //
                             {0}};

int
__enc_diskid_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        if (ENC_XOR == enc->cipher)
        {
            uint32_t medium_id;

            _pages[0].data = enc;
            _pages[0].length = lengthof(_dsn_fields);
            _pages[0].fields = _dsn_fields;
            _dsn_textbox.buffer = enc->data.diskid.dsn;
            _dsn_textbox.capacity = XOR48_DSN_LENGTH;
            _dsn_textbox.length = 0;

            _pages[1].data = enc;
            _pages[1].length = lengthof(_passcode_fields);
            _pages[1].fields = _passcode_fields;
            _passcode_textbox.buffer = enc->buffer;
            _passcode_textbox.capacity = XOR48_PASSCODE_SIZE * 2;
            _passcode_textbox.length = 0;

            if (enc_has_key_store())
            {
                _passcode_fields[3].flags |= SHIZFF_CHECKED;
            }
            else
            {
                _pages[1].length--;
            }

            encui_enter(_pages, 2);

            medium_id = pal_get_medium_id(enc->parameter);
            if (0 != medium_id)
            {
                enc->data.diskid.split.local_part = medium_id;
                shiz_set_page(1);
                return CONTINUE;
            }

            shiz_set_page(0);
            return CONTINUE;
        }

        return -EINVAL;
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
        return IDS_INVALIDDSNPASS;
    }

    case ENCM_GET_STORAGE_POLICY: {
        return (SHIZFF_CHECKED & _passcode_fields[3].flags) ? ENCSTORPOL_SAVE
                                                            : ENCSTORPOL_NONE;
    }
    }

    return -ENOSYS;
}
