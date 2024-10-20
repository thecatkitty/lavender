#include <enc.h>

#include "enc_impl.h"

extern uint64_t
__enc_le32b6d_decode(const void *src);

extern uint64_t
__enc_pkey25xor12_decode(const void *src);

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
