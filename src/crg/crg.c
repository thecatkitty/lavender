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

    switch (cipher)
    {
    case CRG_XOR:
        stream->_impl = &__crg_xor_impl;
        break;

    default:
        return false;
    }

    crg_stream_allocate allocate_impl =
        ((crg_stream_impl *)stream->_impl)->allocate;
    if (NULL == allocate_impl)
    {
        stream->_context = NULL;
        return true;
    }

    return allocate_impl(stream);
}

bool
crg_free(crg_stream *stream)
{
    crg_stream_free free_impl = ((crg_stream_impl *)stream->_impl)->free;
    if (free_impl)
    {
        return free_impl(stream);
    }

    stream->_context = NULL;
    stream->_impl = NULL;
    return true;
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
