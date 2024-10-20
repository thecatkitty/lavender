#ifndef _ENC_IMPL_H_
#define _ENC_IMPL_H_

#include <enc.h>

typedef bool    (*enc_stream_allocate)(enc_stream *stream);
typedef bool    (*enc_stream_free)(enc_stream *stream);
typedef uint8_t (*enc_stream_at)(enc_stream *stream, size_t i);
typedef bool    (*enc_stream_decrypt)(enc_stream *stream, uint8_t *dst);
typedef bool    (*enc_stream_verify)(enc_stream *stream, uint32_t crc);

typedef struct
{
    enc_stream_allocate allocate;
    enc_stream_free     free;
    enc_stream_at       at;
    enc_stream_decrypt  decrypt;
    enc_stream_verify   verify;
} enc_stream_impl;

extern enc_stream_impl __enc_des_impl;
extern enc_stream_impl __enc_xor_impl;

#endif // _ENC_IMPL_H_
