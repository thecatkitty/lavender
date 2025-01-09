#include <stdio.h>

#include "remote.h"

static char _rcode[64];

static encui_field _rcode_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_RCODE_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_LABEL, ENCUIFF_DYNAMIC | ENCUIFF_CENTER},
    {ENCUIFT_SEPARATOR, 0, 2},
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_CCODE_DESC},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&encr_ccode_texbox},
    {ENCUIFT_CHECKBOX, ENCUIFF_STATIC, IDS_STOREKEY},
};

void
encr_rcode_init(enc_context *enc)
{
    encr_pages[PAGE_METHOD].fields[encr_pages[PAGE_METHOD].length++].data =
        IDS_METHOD_RCODE;

    encr_pages[PAGE_RCODE - 1].data = (void *)PAGE_METHOD;
    encr_pages[PAGE_RCODE].data = enc;
    encr_pages[PAGE_RCODE].length = lengthof(_rcode_fields);
    encr_pages[PAGE_RCODE].fields = _rcode_fields;
    encr_pages[PAGE_RCODE].fields[2].data =
        (intptr_t) "888888-888888-888888-888888-888888-888888-888888-"
                   "888888-888888";

    if (enc_has_key_store())
    {
        _rcode_fields[6].flags |= ENCUIFF_CHECKED;
    }
    else
    {
        encr_pages[PAGE_RCODE].length--;
    }
}

int
encr_rcode_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_INIT: {
        uint8_t rbytes[lengthof(encr_request)];
        int     i, offset = 0;

        encr_encode_request(rbytes);
        for (i = 0; i < lengthof(rbytes); i += sizeof(uint16_t))
        {
            unsigned long word = *(uint16_t *)(rbytes + i);
            unsigned long compl10 = encr_decimal_complement(word);
            offset += sprintf(_rcode + offset, "%lu%05lu-", compl10, word);
        }
        _rcode[offset - 1] = 0;

        encr_pages[PAGE_RCODE].fields[2].data = (intptr_t)_rcode;
        return 0;
    }

    case ENCUIM_NEXT: {
        encr_store = ENCUIFF_CHECKED & _rcode_fields[6].flags;
        break;
    }
    }

    return encr_ccode_page_proc(msg, param, data);
}
