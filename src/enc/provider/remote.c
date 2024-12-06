#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <pal.h>

#include "../../resource.h"
#include "../enc_impl.h"
#include "../ui/encui.h"

#define ID_RCODE 1
#define ID_CCODE 2

static encui_page _pages[3];
static uint8_t    _rbytes[18];
static char       _rcode[64];
static uint8_t    _cbytes[14];
static char       _ccode[50];

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
_get_rcode(uint32_t cid, const uint8_t *uid, uint32_t stamp)
{
    int i, offset = 0;

    memcpy(_rbytes + 0, &stamp, 4);
    memcpy(_rbytes + 4, &cid, 4);
    memcpy(_rbytes + 8, uid, 10);

    for (i = 1; i < lengthof(_rbytes); i++)
    {
        _rbytes[i] ^= _rbytes[i - 1];
    }

    for (i = 0; i < lengthof(_rbytes); i += sizeof(uint16_t))
    {
        unsigned long word = *(uint16_t *)(_rbytes + i);
        unsigned long compl10 = _compl10(word);
        offset += sprintf(_rcode + offset, "%lu%05lu-", compl10, word);
    }
    _rcode[offset - 1] = 0;
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
        uint8_t      uid[10];
        time_t       stamp;

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

        stamp = time(NULL);
        _get_rcode(cid, uid, stamp);
        _pages[1].cpx.fields[3].data = (intptr_t)_rcode;

        return 0;
    }
    }

    return -ENOSYS;
}

static int
_ccode_page_proc(int msg, void *param, void *data)
{
    switch (msg)
    {
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
                ((encui_prompt_page *)_pages[1].cpx.fields[6].data)->alert = msg;
                return INT_MAX;
            }

            cwords[i] = (uint16_t)(group % 100000);
        }

        for (i = 0; i < lengthof(_cbytes); i++)
        {
            _cbytes[i] ^= _rbytes[i] ^ _rbytes[i + 2] ^ _rbytes[i + 4];
        }

        return __enc_decrypt_content((enc_context *)data);
    }
    }

    return -ENOSYS;
}

static encui_prompt_page _acode_prompt = {NULL};

static encui_field _acode_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_ENTERPKEY_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&_acode_prompt},
};

static encui_prompt_page _ccode_prompt = {_ccode, sizeof(_ccode), 0};

static encui_field _unlock_fields[] = {
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_UNLOCK_DESC},
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_RCODE_DESC},
    {ENCUIFT_SEPARATOR, 0, 1},
    {ENCUIFT_LABEL, ENCUIFF_DYNAMIC | ENCUIFF_CENTER, ID_RCODE},
    {ENCUIFT_SEPARATOR, 0, 2},
    {ENCUIFT_LABEL, ENCUIFF_STATIC, IDS_CCODE_DESC},
    {ENCUIFT_TEXTBOX, 0, (intptr_t)&_ccode_prompt},
    {ENCUIFT_CHECKBOX, ENCUIFF_STATIC, IDS_STOREKEY},
};

static encui_page _pages[] = {
    {IDS_ENTERPKEY, 0, _acode_page_proc},
    {IDS_UNLOCK, 0, _ccode_page_proc},
    {0}};

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

        _pages[0].data = enc;
        _pages[0].cpx.length = lengthof(_acode_fields);
        _pages[0].cpx.fields = _acode_fields;
        _acode_prompt.buffer = enc->buffer;
        _acode_prompt.capacity = 5 * 5 + 4;
        _acode_prompt.length = 0;

        _pages[1].data = enc;
        _pages[1].cpx.length = lengthof(_unlock_fields);
        _pages[1].cpx.fields = _unlock_fields;
        _pages[1].cpx.fields[3].data =
            (intptr_t) "888888-888888-888888-888888-888888-888888-888888-"
                       "888888-888888";

        if (enc_has_key_store())
        {
            _unlock_fields[7].flags |= ENCUIFF_CHECKED;
        }
        else
        {
            _pages[1].cpx.length--;
        }

        encui_enter(_pages, 2);
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
        return (ENCUIFF_CHECKED & _unlock_fields[7].flags) ? ENCSTORPOL_SAVE
                                                           : ENCSTORPOL_NONE;
    }
    }

    return -ENOSYS;
}
