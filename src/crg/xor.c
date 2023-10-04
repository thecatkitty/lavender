#include <crg.h>
#include <fmt/zip.h>

#include "crg_impl.h"

static uint8_t
_xor_at(crg_stream *stream, size_t i)
{
    return stream->data[i] ^ stream->key[i % stream->key_length];
}

static bool
_xor_decrypt(crg_stream *stream, uint8_t *dst)
{
    for (int i = 0; i < stream->data_length; i++)
    {
        dst[i] = stream->data[i] ^ stream->key[i % stream->key_length];
    }

    return true;
}

static bool
_xor_validate(crg_stream *stream, uint32_t crc)
{
    return crc ==
           zip_calculate_crc_indirect((uint8_t(*)(void *, size_t))_xor_at,
                                      stream, stream->data_length);
}

crg_stream_impl __crg_xor_impl = {
    .at = _xor_at, .decrypt = _xor_decrypt, .validate = _xor_validate};
