#include <assert.h>
#include <stdio.h>
#include <time.h>

#include <pal.h>

#include "remote.h"

uint8_t encr_request[ENCR_REQUEST_SIZE];
uint8_t encr_response[ENCR_RESPONSE_SIZE];
bool    encr_store = false;

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

unsigned long
encr_decimal_complement(unsigned long n)
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
_stamp_request(void)
{
    uint32_t stamp = time(NULL);
    memcpy(encr_request + 0, &stamp, 4);
}

void
encr_encode_request(uint8_t *out)
{
    int i;

    memcpy(out, encr_request, sizeof(encr_request));
    for (i = 1; i < lengthof(encr_request); i++)
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
        uint8_t     *uid = encr_request + 8;

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
        memcpy(encr_request + 4, &cid, 4);

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

        if (2 + 1 == encr_pages[PAGE_METHOD].length)
        {
            // Only the request code method is available
            encr_pages[PAGE_RCODE - 1].data = (void *)PAGE_PKEY;
            _stamp_request();
            encui_set_page(PAGE_RCODE);
            return -EINTR;
        }

        return 0;
    }
    }

    return -ENOSYS;
}

// ----- Method selection

static encui_field _method_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_METHOD_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_LABEL, ENCUIFF_DYNAMIC | ENCUIFF_FOOTER, (intptr_t)""},
    {ENCUIFT_OPTION, ENCUIFF_STATIC | ENCUIFF_CHECKED},
    {ENCUIFT_OPTION, ENCUIFF_STATIC},
#if defined(CONFIG_INTERNET)
    {ENCUIFT_OPTION, ENCUIFF_STATIC},
#endif
};

static int
_method_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
    case ENCUIM_INIT: {
#if defined(CONFIG_INTERNET)
        // in case when navigated back from PAGE_INET
        encr_inet_cleanup();
#endif
        return 0;
    }

    case ENCUIM_NEXT: {
        encui_field *it = encr_pages[PAGE_METHOD].fields + 3;
        encui_field *end =
            encr_pages[PAGE_METHOD].fields + encr_pages[PAGE_METHOD].length;

        while ((it < end) && (0 == (ENCUIFF_CHECKED & it->flags)))
        {
            it++;
        }
        assert(it < end);

        _stamp_request();

#if defined(CONFIG_INTERNET)
        if (IDS_METHOD_INET == it->data)
        {
            encui_set_page(PAGE_INET);
            return -EINTR;
        }
#endif

        if (IDS_METHOD_QR == it->data)
        {
            encr_qr_enter(data);
            return -EINTR;
        }

        if (IDS_METHOD_RCODE == it->data)
        {
            encui_set_page(PAGE_RCODE);
            return -EINTR;
        }
    }

#if defined(_WIN32) || defined(__linux__)
    case ENCUIM_NOTIFY: {
        if (0x102 == (intptr_t)param)
        {
            enc_context *enc = (enc_context *)data;
            char         url[260];
            snprintf(url, lengthof(url), "%s/privacy", (char *)enc->parameter);
            pal_open_url(url);
        }

        return 0;
    }
#endif
    }

    return -ENOSYS;
}

// ----- Pages

encui_page encr_pages[] = {
    {IDS_ENTERPKEY, _acode_page_proc},  // PAGE_PKEY
    {IDS_METHOD, _method_page_proc},    // PAGE_METHOD
    {0},                                //
    {IDS_UNLOCK, encr_rcode_page_proc}, // PAGE_RCODE
    {0},                                //
    {IDS_UNLOCK, encr_qr_page_proc},    // PAGE_QR
#if defined(CONFIG_INTERNET)
    {0},                               //
    {IDS_UNLOCK, encr_inet_page_proc}, // PAGE_INET
#endif
    {0} //
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

        encr_pages[PAGE_PKEY].data = enc;
        encr_pages[PAGE_PKEY].length = lengthof(_acode_fields);
        encr_pages[PAGE_PKEY].fields = _acode_fields;
        _acode_textbox.buffer = enc->buffer;
        _acode_textbox.capacity = 5 * 5 + 4;
        _acode_textbox.length = 0;

        encr_pages[PAGE_METHOD].data = enc;
        encr_pages[PAGE_METHOD].length = 3;
        encr_pages[PAGE_METHOD].fields = _method_fields;

#if defined(CONFIG_INTERNET)
        encr_inet_init(enc);
#endif
        encr_qr_init(enc);
        encr_rcode_init(enc);

        encui_enter(encr_pages, PAGE_LAST + 1);
        encui_set_page(0);
        return CONTINUE;
    }

    case ENCM_TRANSFORM: {
        __enc_des_expand56(_56betoull(encr_response), enc->key.b);
        __enc_des_expand56(_56betoull(encr_response + 7), enc->key.b + 8);
        (&enc->key.qw)[0] = BSWAP64((&enc->key.qw)[0]);
        (&enc->key.qw)[1] = BSWAP64((&enc->key.qw)[1]);
        return 0;
    }

    case ENCM_GET_ERROR_STRING: {
#if defined(CONFIG_INTERNET)
        if (200 == encr_inet_get_status())
        {
            return IDS_INET_CODEERR;
        }
#endif
        return IDS_INVALIDCCODE;
    }

    case ENCM_GET_STORAGE_POLICY: {
        return encr_store ? ENCSTORPOL_SAVE : ENCSTORPOL_NONE;
    }
    }

    return -ENOSYS;
}
