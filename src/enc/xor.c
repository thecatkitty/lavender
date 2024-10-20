#include <enc.h>
#include <fmt/zip.h>

#include "enc_impl.h"

static uint8_t
_xor_at(enc_stream *stream, size_t i)
{
    return stream->data[i] ^ stream->key[i % stream->key_length];
}

static bool
_xor_decrypt(enc_stream *stream, uint8_t *dst)
{
    for (int i = 0; i < stream->data_length; i++)
    {
        dst[i] = stream->data[i] ^ stream->key[i % stream->key_length];
    }

    return true;
}

static bool
_xor_validate(enc_stream *stream, uint32_t crc)
{
    return crc ==
           zip_calculate_crc_indirect((uint8_t(*)(void *, size_t))_xor_at,
                                      stream, stream->data_length);
}

enc_stream_impl __enc_xor_impl = {
    .at = _xor_at, .decrypt = _xor_decrypt, .validate = _xor_validate};
