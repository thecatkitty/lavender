#include <crg.h>

#include "crg_impl.h"

bool
crg_prepare(crg_stream    *stream,
            crg_cipher     cipher,
            const uint8_t *data,
            size_t         data_length,
            const uint8_t *key,
            size_t         key_length)
{
    stream->data = data;
    stream->data_length = data_length;
    stream->key = key;
    stream->key_length = key_length;

    if (CRG_XOR == cipher)
    {
        stream->_impl = &__crg_xor_impl;
        return true;
    }

    return false;
}

uint8_t
crg_at(crg_stream *stream, size_t i)
{
    return ((const crg_stream_impl *)stream->_impl)->at(stream, i);
}

bool
crg_decrypt(crg_stream *stream, uint8_t *dst)
{
    return ((const crg_stream_impl *)stream->_impl)->decrypt(stream, dst);
}

bool
crg_validate(crg_stream *stream, uint32_t crc)
{
    return ((const crg_stream_impl *)stream->_impl)->validate(stream, crc);
}
