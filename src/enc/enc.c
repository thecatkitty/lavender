#include <enc.h>
#include <fmt/zip.h>

#include "enc_impl.h"
#include "ui/encui.h"

// Size of the plaintext stored at the end of the data buffer
#define PT_SIZE(data, size)                                                    \
    (*(uint32_t *)((char *)(data) + (size) - sizeof(uint32_t)))

const enc_provider_impl *PROVIDER[] = {
    [ENC_PROVIDER_CALLER] = &__enc_caller_impl,
    [ENC_PROVIDER_PROMPT] = &__enc_prompt_impl,
    [ENC_PROVIDER_SPLIT] = &__enc_split_impl,
    [ENC_PROVIDER_DISKID] = &__enc_diskid_impl,
};

extern uint64_t
__enc_le32b6d_decode(const void *src);

extern uint64_t
__enc_pkey25xor12_decode(const void *src);

extern bool
__enc_pkey25xor12_validate_format(const char *key);

bool
enc_prepare(enc_stream    *stream,
            enc_cipher     cipher,
            const uint8_t *data,
            size_t         data_length,
            const uint8_t *key,
            size_t         key_length)
{
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
        stream->_impl = &__enc_des_impl;
        break;

    default:
        return false;
    }

    enc_stream_allocate allocate_impl =
        ((enc_stream_impl *)stream->_impl)->allocate;
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

uint64_t
enc_decode_key(const void *src, enc_keysm sm)
{
    errno = 0;

    if (sm == ENC_KEYSM_LE32B6D)
    {
        return __enc_le32b6d_decode(src);
    }

    if (sm == ENC_KEYSM_PKEY25XOR12)
    {
        return __enc_pkey25xor12_decode(src);
    }

    errno = EFTYPE;
    return 0;
}

bool
enc_validate_key_format(const char *key, enc_keysm sm)
{
    if (ENC_KEYSM_PKEY25XOR12 == sm)
    {
        return __enc_pkey25xor12_validate_format(key);
    }

    return false;
}

int
enc_handle(enc_context *enc)
{
    switch (enc->state)
    {
    case ENCS_ACQUIRE: {
        int status = PROVIDER[enc->provider & 0xFF]->acquire(enc);
        if (0 > status)
        {
            encui_exit();
            return status;
        }

        if (0 == status)
        {
            enc->state = ENCS_VERIFY;
        }

        return CONTINUE;
    }

    case ENCS_VERIFY: {
        if (!enc_prepare(&enc->stream, enc->cipher, enc->content, enc->size,
                         enc->key.b,
                         (ENC_XOR == enc->cipher) ? 6 : sizeof(uint64_t)))
        {
            return -EINVAL;
        }

        if (!enc_verify(&enc->stream, enc->crc32))
        {
            enc->state = ENCS_INVALID;
            enc_free(&enc->stream);
            return CONTINUE;
        }

        enc_decrypt(&enc->stream, (uint8_t *)enc->content);
        enc_free(&enc->stream);
        if (ENC_DES == enc->cipher)
        {
            PT_SIZE(enc->content, enc->size) = enc->stream.data_length;
            enc->size = enc->stream.data_length;
        }

        enc->state = ENCS_COMPLETE;
        return CONTINUE;
    }

    case ENCS_COMPLETE:
        encui_exit();
        return 0;
    }

    return PROVIDER[enc->provider & 0xFF]->handle(enc);
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

    if (ENC_DES == cipher)
    {
        uint32_t pt_size = PT_SIZE(content, length);
        if ((length > pt_size) && zip_calculate_crc(content, pt_size) == crc32)
        {
            enc->size = pt_size;
            enc->state = ENCS_COMPLETE;
            return 0;
        }
    }

    enc->state = ENCS_ACQUIRE;
    return CONTINUE;
}
