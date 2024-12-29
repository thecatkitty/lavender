#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

#include "../../../ext/QR-Code-generator/c/qrcodegen.h"

#define ID_RCODE 1
#define ID_CCODE 2

enum
{
    PAGE_PKEY,
    PAGE_METHOD,
    PAGE_RCODE,
    PAGE_QR = PAGE_RCODE + 2,
};

static encui_page _pages[6];
static uint8_t    _rbytes[18];
static char       _rcode[64];
static uint8_t    _cbytes[14];
static char       _ccode[50];
static gfx_bitmap _qr_bitmap = {128, 128, 16, 1, 1};
static bool       _save = false;

static int
_parity(uint64_t n)
{
#if defined(_MSC_VER)
    uint8_t ret = 0;

    while (n)
    {
        ret ^= (n & 1);
        n >>= 1;
    }

    return ret;
#else
    return __builtin_parityll(n);
#endif
}

static unsigned long
_compl10(unsigned long n)
{
    unsigned long sum = 0;
    while (n)
    {
        sum += n % 10;
        n /= 10;
    }

    return (10 - (sum % 10)) % 10;
}

static void
_stamp_rbytes(void)
{
    uint32_t stamp = time(NULL);
    memcpy(_rbytes + 0, &stamp, 4);
}

static void
_encode_rbytes(uint8_t* out)
{
    int i;

    memcpy(out, _rbytes, sizeof(_rbytes));
    for (i = 1; i < lengthof(_rbytes); i++)
    {
        out[i] ^= out[i - 1];
    }
}

// ----- Access code input

static encui_textbox_data _acode_textbox = {NULL};

static encui_field _acode_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_ENTERPKEY_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&_acode_textbox},
};

static void
_set_pixel(gfx_bitmap *bm, int x, int y, int scale, bool value)
{
    uint8_t *line = (uint8_t *)bm->bits + y * scale * bm->opl;
    int      sx, sy;

    for (sy = 0; sy < scale; sy++)
    {
        for (sx = 0; sx < scale; sx++)
        {
            uint8_t *cell = line + (x * scale + sx) / 8;
            if (value)
            {
                *cell &= ~(0x80 >> ((x * scale + sx) % 8));
            }
        }

        line += bm->opl;
    }
}

static bool
_get_qr(const char* str, gfx_bitmap* bm)
{
    uint8_t buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];
    uint8_t qr[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];
    int     size, scale, x, y;

    if (!qrcodegen_encodeText(str, buffer, qr, qrcodegen_Ecc_MEDIUM, 1, 10,
                              qrcodegen_Mask_AUTO, true))
    {
        return false;
    }

    if (NULL == _qr_bitmap.bits)
    {
        _qr_bitmap.bits = malloc(bm->height * bm->opl);
    }

    if (NULL == _qr_bitmap.bits)
    {
        return false;
    }

    size = qrcodegen_getSize(qr);
    scale = bm->width / size;

    memset(bm->bits, 0xFF, bm->opl * bm->height);
    for (y = 0; y < size; y++)
    {
        for (x = 0; x < size; x++)
        {
            _set_pixel(bm, x, y, scale, qrcodegen_getModule(qr, x, y));
        }
    }

    return true;
}

static int
_acode_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_CHECK: {
        const char *acode = (const char *)param;
        if (NULL == acode)
        {
            return 1;
        }

        return enc_validate_key_format(acode, ENC_KEYSM_PKEY25RAW) ? 0 : 1;
    }

    case ENCUIM_NEXT: {
        enc_context *enc = (enc_context *)data;
        uint64_t     parts[2] = {0, 0};
        uint32_t     cid;
        uint64_t     uid_left, uid_right;
        uint8_t     *uid = _rbytes + 8;

        enc_decode_key(enc->buffer, parts, ENC_KEYSM_PKEY25RAW);
        if (_parity(parts[0]) || _parity(parts[1]))
        {
            return IDS_INVALIDPKEY;
        }

        parts[0] >>= 1; // ((uid >> 38) << 16) | (xcid >> 16)
        parts[1] >>= 1; // ((uid & 0x3FFFFFFFFF) << 16) | (xcid & 0xFFFF)

        uid_left = parts[0] >> 16;  // 42 bits, uid >> 38
        uid_right = parts[1] >> 16; // 38 bits, uid & 0x3FFFFFFFFF

        uid_right |= (uid_left & 0x3FF) << 38; // 48 bits
        uid_left >>= 10;                       // 32 bits

        cid = ((parts[0] & 0xFFFF) << 16) | (parts[1] & 0xFFFF);
        cid ^= (uid_right >> 12) & 0xFFFFFFFF;
        if (enc->crc32 != cid)
        {
            return IDS_WRONGCID;
        }
        memcpy(_rbytes + 4, &cid, 4);

        // 80-bit user ID in big endian
        uid[0] = (uid_left >> 24) & 0xFF;
        uid[1] = (uid_left >> 16) & 0xFF;
        uid[2] = (uid_left >> 8) & 0xFF;
        uid[3] = (uid_left >> 0) & 0xFF;
        uid[4] = (uid_right >> 40) & 0xFF;
        uid[5] = (uid_right >> 32) & 0xFF;
        uid[6] = (uid_right >> 24) & 0xFF;
        uid[7] = (uid_right >> 16) & 0xFF;
        uid[8] = (uid_right >> 8) & 0xFF;
        uid[9] = (uid_right >> 0) & 0xFF;

        return 0;
    }
    }

    return -ENOSYS;
}

// ----- Method selection

static encui_field _method_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_METHOD_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_OPTION, ENCUIFF_STATIC | ENCUIFF_CHECKED},
    {ENCUIFT_OPTION, ENCUIFF_STATIC},
};

static int
_method_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_NEXT: {
        encui_field *it = _pages[PAGE_METHOD].fields;
        encui_field *end = it + _pages[PAGE_METHOD].length;

        while ((it < end) && (0 == (ENCUIFF_CHECKED & it->flags)))
        {
            it++;
        }
        assert(it < end);

        _stamp_rbytes();

        if (IDS_METHOD_QR == it->data)
        {
            enc_context *enc = (enc_context *)data;
            uint8_t      rbytes[lengthof(_rbytes)];
            char         url[100] = "", *purl;
            size_t       url_len = strlen(enc->parameter);
            int          i;

            strcpy(url, enc->parameter);
            strcpy(url + url_len, "/qr?rc=");
            purl = url + url_len + 7;

            _encode_rbytes(rbytes);
            for (i = 0; i < lengthof(rbytes); i++)
            {
                sprintf(purl, "%02x", rbytes[i]);
                purl += 2;
            }

            _get_qr(url, &_qr_bitmap);
            encui_set_page(PAGE_QR);

            free(_qr_bitmap.bits);
            _qr_bitmap.bits = NULL;
            return -EINTR;
        }

        if (IDS_METHOD_RCODE == it->data)
        {
            encui_set_page(PAGE_RCODE);
            return -EINTR;
        }
    }
    }

    return -ENOSYS;
}

// ----- Request code with manual delivery, QR code

static encui_textbox_data _ccode_textbox = {_ccode, sizeof(_ccode), 0};

static encui_field _rcode_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_UNLOCK_DESC},
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_RCODE_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_LABEL, ENCUIFF_DYNAMIC | ENCUIFF_CENTER, ID_RCODE},
    {ENCUIFT_SEPARATOR, 0, 2},
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_CCODE_DESC},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&_ccode_textbox},
    {ENCUIFT_CHECKBOX, ENCUIFF_STATIC, IDS_STOREKEY},
};

static encui_field _qr_fields[] = {
    {ENCUIFT_BITMAP, ENCUIFF_DYNAMIC | ENCUIFF_CENTER, (intptr_t)&_qr_bitmap},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_QR_DESC},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&_ccode_textbox},
    {ENCUIFT_CHECKBOX, ENCUIFF_STATIC, IDS_STOREKEY},
};

static void
_get_rcode(void)
{
    uint8_t rbytes[lengthof(_rbytes)];
    int i, offset = 0;

    _encode_rbytes(rbytes);
    for (i = 0; i < lengthof(rbytes); i += sizeof(uint16_t))
    {
        unsigned long word = *(uint16_t *)(rbytes + i);
        unsigned long compl10 = _compl10(word);
        offset += sprintf(_rcode + offset, "%lu%05lu-", compl10, word);
    }
    _rcode[offset - 1] = 0;
}

static int
_ccode_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_INIT: {
        _get_rcode();
        _pages[PAGE_RCODE].fields[3].data = (intptr_t)_rcode;
        return 0;
    }

    case ENCUIM_CHECK: {
        int i;

        const char *ccode = (const char *)param;
        if (NULL == ccode)
        {
            return 1;
        }

        if (48 != strlen(ccode))
        {
            return 1;
        }

        for (i = 0; i < 48; i++)
        {
            if (6 == (i % 7))
            {
                if ('-' != ccode[i])
                {
                    return 1;
                }
            }
            else if (!isdigit(ccode[i]))
            {
                return 1;
            }
        }

        return 0;
    }

    case ENCUIM_NEXT: {
        int         i;
        const char *ccode = (const char *)param;
        uint16_t   *cwords = (uint16_t *)_cbytes;
        uint8_t     rbytes[lengthof(_rbytes)];

        for (i = 0; i < lengthof(_cbytes) / 2; i++)
        {
            uint32_t group = atol(ccode + i * 7);
            if (0 != _compl10(group))
            {
                char  fmt[GFX_COLUMNS * 2];
                char *msg;
                int   length;

                pal_load_string(IDS_INVALIDGROUP, fmt, lengthof(fmt));
                length = snprintf(NULL, 0, fmt, i + 1);
                msg = malloc(length + 1);
                snprintf(msg, length + 1, fmt, i + 1);
                ((encui_textbox_data *)_pages[PAGE_RCODE].fields[6].data)
                    ->alert = msg;
                return INT_MAX;
            }

            cwords[i] = (uint16_t)(group % 100000);
        }

        _encode_rbytes(rbytes);
        for (i = 0; i < lengthof(_cbytes); i++)
        {
            _cbytes[i] ^= rbytes[i] ^ rbytes[i + 2] ^ rbytes[i + 4];
        }

        if (PAGE_RCODE == encui_get_page())
        {
            _save = ENCUIFF_CHECKED & _rcode_fields[7].flags;
        }
        else
        {
            _save = ENCUIFF_CHECKED & _qr_fields[4].flags;
        }

        return __enc_decrypt_content((enc_context *)data);
    }
    }

    return -ENOSYS;
}

// ----- Pages

static encui_page _pages[] = {
    {IDS_ENTERPKEY, _acode_page_proc}, // PAGE_PKEY
    {IDS_METHOD, _method_page_proc},   // PAGE_METHOD
    {IDS_UNLOCK, _ccode_page_proc},    // PAGE_RCODE
    {0},                               //
    {IDS_UNLOCK, _ccode_page_proc},    // PAGE_QR
    {0}                                //
};

static uint64_t
_56betoull(const uint8_t *src)
{
    int      i;
    uint64_t ret = 0;

    for (i = 0; i < 7; i++)
    {
        ret <<= 8;
        ret |= src[i];
    }

    return ret;
}

int
__enc_remote_proc(int msg, enc_context *enc)
{
    switch (msg)
    {
    case ENCM_INITIALIZE: {
        if (ENC_TDES != enc->cipher)
        {
            return -EINVAL;
        }

        enc->stream.key_length = 2 * sizeof(uint64_t);

        _pages[PAGE_PKEY].data = enc;
        _pages[PAGE_PKEY].length = lengthof(_acode_fields);
        _pages[PAGE_PKEY].fields = _acode_fields;
        _acode_textbox.buffer = enc->buffer;
        _acode_textbox.capacity = 5 * 5 + 4;
        _acode_textbox.length = 0;

        _pages[PAGE_METHOD].data = enc;
        _pages[PAGE_METHOD].length = 2;
        _pages[PAGE_METHOD].fields = _method_fields;
        if (NULL != enc->parameter)
        {
            _pages[PAGE_METHOD].fields[_pages[PAGE_METHOD].length++].data =
                IDS_METHOD_QR;
        }
        _pages[PAGE_METHOD].fields[_pages[PAGE_METHOD].length++].data =
            IDS_METHOD_RCODE;

        _pages[PAGE_RCODE].data = enc;
        _pages[PAGE_RCODE].length = lengthof(_rcode_fields);
        _pages[PAGE_RCODE].fields = _rcode_fields;
        _pages[PAGE_RCODE].fields[3].data =
            (intptr_t) "888888-888888-888888-888888-888888-888888-888888-"
                       "888888-888888";

        _pages[PAGE_QR - 1].data = (void *)PAGE_METHOD;
        _pages[PAGE_QR].data = enc;
        _pages[PAGE_QR].length = lengthof(_qr_fields);
        _pages[PAGE_QR].fields = _qr_fields;

        if (enc_has_key_store())
        {
            _rcode_fields[7].flags |= ENCUIFF_CHECKED;
            _qr_fields[4].flags |= ENCUIFF_CHECKED;
        }
        else
        {
            _pages[PAGE_RCODE].length--;
            _pages[PAGE_QR].length--;
        }

        encui_enter(_pages, lengthof(_pages));
        encui_set_page(0);
        return CONTINUE;
    }

    case ENCM_TRANSFORM: {
        __enc_des_expand56(_56betoull(_cbytes), enc->key.b);
        __enc_des_expand56(_56betoull(_cbytes + 7), enc->key.b + 8);
        (&enc->key.qw)[0] = __builtin_bswap64((&enc->key.qw)[0]);
        (&enc->key.qw)[1] = __builtin_bswap64((&enc->key.qw)[1]);
        return 0;
    }

    case ENCM_GET_ERROR_STRING: {
        return IDS_INVALIDCCODE;
    }

    case ENCM_GET_STORAGE_POLICY: {
        return _save ? ENCSTORPOL_SAVE : ENCSTORPOL_NONE;
    }
    }

    return -ENOSYS;
}
