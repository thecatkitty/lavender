#ifndef _CRG_IMPL_H_
#define _CRG_IMPL_H_

#include <crg.h>

typedef bool    (*crg_stream_allocate)(crg_stream *stream);
typedef bool    (*crg_stream_free)(crg_stream *stream);
typedef uint8_t (*crg_stream_at)(crg_stream *stream, size_t i);
typedef bool    (*crg_stream_decrypt)(crg_stream *stream, uint8_t *dst);
typedef bool    (*crg_stream_validate)(crg_stream *stream, uint32_t crc);

typedef struct
{
    crg_stream_allocate allocate;
    crg_stream_free     free;
    crg_stream_at       at;
    crg_stream_decrypt  decrypt;
    crg_stream_validate validate;
} crg_stream_impl;

extern crg_stream_impl __crg_des_impl;
extern crg_stream_impl __crg_xor_impl;

#endif // _CRG_IMPL_H_
