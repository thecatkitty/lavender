#include <enc.h>
#include <fmt/zip.h>

#include "../enc_impl.h"

static uint8_t
_xor_at(enc_stream *stream, size_t i)
{
    return stream->data[i] ^ stream->key[i % stream->key_length];
}

static bool
_xor_decrypt(enc_stream *stream, uint8_t *dst)
{
    int i;
    for (i = 0; i < stream->data_length; i++)
    {
        dst[i] = stream->data[i] ^ stream->key[i % stream->key_length];
    }

    return true;
}

static bool
_xor_verify(enc_stream *stream, uint32_t crc)
{
    return crc ==
           zip_calculate_crc_indirect((uint8_t(*)(void *, size_t))_xor_at,
                                      stream, stream->data_length);
}

enc_stream_impl __enc_xor_impl = {NULL, NULL, _xor_at, _xor_decrypt,
                                  _xor_verify};
