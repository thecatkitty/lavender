#include <stdio.h>

#include "qr/encqr.h"
#include "remote.h"

#define QR_SIZE 128

static gfx_bitmap _qr_bitmap = {QR_SIZE, QR_SIZE, QR_SIZE / 8, 1, 1};

static encui_field _qr_fields[] = {
    {ENCUIFT_BITMAP, ENCUIFF_DYNAMIC | ENCUIFF_CENTER, (intptr_t)&_qr_bitmap},
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_QR_DESC},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&encr_ccode_texbox},
    {ENCUIFT_CHECKBOX, ENCUIFF_STATIC, IDS_STOREKEY},
};

void
encr_qr_init(enc_context *enc)
{
    if (NULL == enc->parameter)
    {
        return;
    }

    encr_pages[PAGE_METHOD].fields[encr_pages[PAGE_METHOD].length++].data =
        IDS_METHOD_QR;

    encr_pages[PAGE_QR - 1].data = (void *)PAGE_METHOD;
    encr_pages[PAGE_QR].data = enc;
    encr_pages[PAGE_QR].length = lengthof(_qr_fields);
    encr_pages[PAGE_QR].fields = _qr_fields;

    if (enc_has_key_store())
    {
        _qr_fields[3].flags |= ENCUIFF_CHECKED;
    }
    else
    {
        encr_pages[PAGE_QR].length--;
    }
}

void
encr_qr_enter(void *data)
{
    enc_context *enc = (enc_context *)data;
    uint8_t      rbytes[lengthof(encr_request)];
    char         url[100] = "", *purl;
    size_t       url_len = strlen(enc->parameter);
    int          i;

    strcpy(url, enc->parameter);
    strcpy(url + url_len, "/qr?rc=");
    purl = url + url_len + 7;

    encr_encode_request(rbytes);
    for (i = 0; i < lengthof(rbytes); i++)
    {
        sprintf(purl, "%02x", rbytes[i]);
        purl += 2;
    }

    _qr_bitmap.width = _qr_bitmap.height = QR_SIZE;
    if (NULL == _qr_bitmap.bits)
    {
        _qr_bitmap.bits = malloc(_qr_bitmap.height * _qr_bitmap.opl);
    }
    encqr_generate(url, &_qr_bitmap);
    _qr_bitmap.width = (_qr_bitmap.width + 7) / 8 * 8;

    encui_set_page(PAGE_QR);

    free(_qr_bitmap.bits);
    _qr_bitmap.bits = NULL;
}

int
encr_qr_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_NEXT: {
        encr_store = ENCUIFF_CHECKED & _qr_fields[3].flags;
        break;
    }
    }

    return encr_ccode_page_proc(msg, param, data);
}
