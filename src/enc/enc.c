#include <stdio.h>

#include <enc.h>
#include <fmt/zip.h>
#include <pal.h>

#include "enc_impl.h"
#include "ui/encui.h"

// Size of the plaintext stored at the end of the data buffer
#define PT_SIZE(data, size)                                                    \
    (*(uint32_t *)((char *)(data) + (size) - sizeof(uint32_t)))

static const char CONTENT_KEY_FMT[] = "ContentKey-%08" PRIx32;

static enc_provider_proc *const PROVIDER[] = {
    &__enc_caller_proc,
    &__enc_prompt_proc,
    &__enc_split_proc,
    &__enc_diskid_proc,
    &__enc_remote_proc,
};

#define ENC_PROV(enc) (*(PROVIDER[(enc)->provider & 0xFF]))

extern int
__enc_le32b6d_decode(const void *src, void *dst);

extern int
__enc_pkey25raw_decode(const void *src, void *dst);

extern int
__enc_pkey25xor12_decode(const void *src, void *dst);

extern int
__enc_pkey25xor2b_decode(const void *src, void *dst);

extern bool
__enc_pkey25_validate_format(const char *key);

static int
_decrypt_content(enc_context *enc, bool load_key);

bool
enc_prepare(enc_stream    *stream,
            enc_cipher     cipher,
            const uint8_t *data,
            size_t         data_length,
            const uint8_t *key,
            size_t         key_length)
{
    enc_stream_allocate allocate_impl = NULL;

    stream->data = data;
    stream->data_length = data_length;
    stream->key = key;
    stream->key_length = key_length;

    switch (cipher)
    {
    case ENC_XOR:
        stream->_impl = &__enc_xor_impl;
        break;

    case ENC_DES:
    case ENC_TDES:
        stream->_impl = &__enc_des_impl;
        break;

    default:
        return false;
    }

    allocate_impl = ((enc_stream_impl *)stream->_impl)->allocate;
    if (NULL == allocate_impl)
    {
        stream->_context = NULL;
        return true;
    }

    return allocate_impl(stream);
}

bool
enc_free(enc_stream *stream)
{
    enc_stream_free free_impl = ((enc_stream_impl *)stream->_impl)->free;
    if (free_impl)
    {
        return free_impl(stream);
    }

    stream->_context = NULL;
    stream->_impl = NULL;
    return true;
}

uint8_t
enc_at(enc_stream *stream, size_t i)
{
    return ((const enc_stream_impl *)stream->_impl)->at(stream, i);
}

bool
enc_decrypt(enc_stream *stream, uint8_t *dst)
{
    return ((const enc_stream_impl *)stream->_impl)->decrypt(stream, dst);
}

bool
enc_verify(enc_stream *stream, uint32_t crc)
{
    return ((const enc_stream_impl *)stream->_impl)->verify(stream, crc);
}

int
enc_decode_key(const void *src, void *dst, enc_keysm sm)
{
    if (sm == ENC_KEYSM_LE32B6D)
    {
        return __enc_le32b6d_decode(src, dst);
    }

    if (sm == ENC_KEYSM_PKEY25RAW)
    {
        return __enc_pkey25raw_decode(src, dst);
    }

    if (sm == ENC_KEYSM_PKEY25XOR12)
    {
        return __enc_pkey25xor12_decode(src, dst);
    }

    if (sm == ENC_KEYSM_PKEY25XOR2B)
    {
        return __enc_pkey25xor2b_decode(src, dst);
    }

    return -(errno = EFTYPE);
}

bool
enc_validate_key_format(const char *key, enc_keysm sm)
{
    if ((ENC_KEYSM_PKEY25RAW == sm) || (ENC_KEYSM_PKEY25XOR12 == sm) ||
        (ENC_KEYSM_PKEY25XOR2B == sm))
    {
#ifdef __ia16__
        // FIXME: W/A for return value truncation
        // The condition below will never happen, but without this block the
        // returned value gets lost under gcc-ia16.
        if (29 < strlen(key))
        {
            assert(!__enc_pkey25_validate_format(key));
        }
#endif
        return __enc_pkey25_validate_format(key);
    }

    return false;
}

#define REQUIRE_SUCCESS(status)                                                \
    {                                                                          \
        int __status = (status);                                               \
        if (0 > __status)                                                      \
        {                                                                      \
            encui_exit();                                                      \
            return __status;                                                   \
        }                                                                      \
    }

int
enc_handle(enc_context *enc)
{
    int status = 0;

    switch (enc->state)
    {
    case ENCS_INITIALIZE: {
        if (0 == _decrypt_content(enc, true))
        {
            enc->state = ENCS_COMPLETE;
            return CONTINUE;
        }

        REQUIRE_SUCCESS(status = ENC_PROV(enc)(ENCM_INITIALIZE, enc));
        if (0 == status)
        {
            enc->state = ENCS_VERIFY;
            return CONTINUE;
        }

        enc->state = ENCS_READ;
        return CONTINUE;
    }

    case ENCS_READ: {
        int status = encui_handle();
        if (ENCUI_INCOMPLETE == status)
        {
            return CONTINUE;
        }

        if (0 == status)
        {
            // Operation aborted by the user
            encui_exit();
            return -EACCES;
        }

        encui_set_page(encui_get_page() + 1);
        if (-1 == encui_get_page())
        {
            enc->state = ENCS_COMPLETE;
        }

        return CONTINUE;
    }

    case ENCS_VERIFY: {
        REQUIRE_SUCCESS(_decrypt_content(enc, false));
        enc->state = ENCS_COMPLETE;
        return CONTINUE;
    }

    case ENCS_COMPLETE:
        encui_exit();
        return 0;
    }

    return -ENOSYS;
}

static int
_decrypt_content(enc_context *enc, bool load_key)
{
    size_t size = (ENC_XOR == enc->cipher)    ? 6
                  : (ENC_DES == enc->cipher)  ? sizeof(uint64_t)
                  : (ENC_TDES == enc->cipher) ? (2 * sizeof(uint64_t))
                                              : 0;
    if (load_key)
    {
        if (!enc_load_key(enc->crc32, enc->key.b, size))
        {
            return -ENOENT;
        }
    }
    else
    {
        REQUIRE_SUCCESS(ENC_PROV(enc)(ENCM_TRANSFORM, enc));
    }

    if (!enc_prepare(&enc->stream, enc->cipher, enc->content, enc->size,
                     enc->key.b, size))
    {
        return -EINVAL;
    }

    if (!enc_verify(&enc->stream, enc->crc32))
    {
        enc_free(&enc->stream);
        return -EACCES;
    }

    if (!load_key &&
        (ENCSTORPOL_SAVE == ENC_PROV(enc)(ENCM_GET_STORAGE_POLICY, enc)))
    {
        enc_save_key(enc->crc32, enc->key.b, enc->stream.key_length);
    }

    enc_decrypt(&enc->stream, (uint8_t *)enc->content);
    enc_free(&enc->stream);
    if ((ENC_DES == enc->cipher) || (ENC_TDES == enc->cipher))
    {
        PT_SIZE(enc->content, enc->size) = enc->stream.data_length;
        enc->size = enc->stream.data_length;
    }

    return 0;
}

int
__enc_decrypt_content(enc_context *enc)
{
    int status = _decrypt_content(enc, false);
    if ((0 > status) && (-EACCES != status))
    {
        return status;
    }

    if (-EACCES == status)
    {
        return ENC_PROV(enc)(ENCM_GET_ERROR_STRING, enc);
    }

    return 0;
}

int
enc_access_content(enc_context *enc,
                   enc_cipher   cipher,
                   int          provider,
                   const void  *parameter,
                   uint8_t     *content,
                   size_t       length,
                   uint32_t     crc32)
{
    memset(enc, 0, sizeof(*enc));
    enc->cipher = cipher;
    enc->provider = provider;
    enc->parameter = parameter;
    enc->content = content;
    enc->size = length;
    enc->crc32 = crc32;

    if ((ENC_XOR == cipher) && (zip_calculate_crc(content, length) == crc32))
    {
        enc->state = ENCS_COMPLETE;
        return 0;
    }

    if ((ENC_DES == cipher) || (ENC_TDES == cipher))
    {
        uint32_t pt_size = PT_SIZE(content, length);
        if ((length > pt_size) && zip_calculate_crc(content, pt_size) == crc32)
        {
            enc->size = pt_size;
            enc->state = ENCS_COMPLETE;
            return 0;
        }
    }

    enc->state = ENCS_INITIALIZE;
    return CONTINUE;
}

bool
enc_load_key(uint32_t cid, uint8_t *buffer, size_t size)
{
    size_t  i;
    uint8_t mid[PAL_MACHINE_ID_SIZE];
    char    name[20];

    if (!pal_get_machine_id(mid))
    {
        return false;
    }

    snprintf(name, sizeof(name), CONTENT_KEY_FMT, cid);
    if (size != pal_load_state(name, buffer, size))
    {
        return false;
    }

    for (i = 0; i < size; i++)
    {
        buffer[i] ^= mid[i % PAL_MACHINE_ID_SIZE];
    }

    return true;
}

bool
enc_save_key(uint32_t cid, const uint8_t *buffer, size_t size)
{
    uint8_t bound_key[ENC_KEY_LENGTH_MAX];
    size_t  i;
    uint8_t mid[PAL_MACHINE_ID_SIZE];
    char    name[20];

    if (ENC_KEY_LENGTH_MAX < size)
    {
        return false;
    }

    if (!pal_get_machine_id(mid))
    {
        return false;
    }

    for (i = 0; i < size; i++)
    {
        bound_key[i] = buffer[i] ^ mid[i % PAL_MACHINE_ID_SIZE];
    }

    snprintf(name, sizeof(name), CONTENT_KEY_FMT, cid);
    return pal_save_state(name, bound_key, size);
}
