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

typedef int (enc_provider_proc)(int msg, enc_context *enc);

extern enc_stream_impl __enc_des_impl;
extern enc_stream_impl __enc_xor_impl;

extern enc_provider_proc __enc_caller_proc;
extern enc_provider_proc __enc_diskid_proc;
extern enc_provider_proc __enc_split_proc;
extern enc_provider_proc __enc_prompt_proc;

#define CONTINUE 1
#define CUSTOM   2

enum
{
    ENCS_INITIALIZE,
    ENCS_CUSTOM,
    ENCS_ACQUIRE,
    ENCS_READ,
    ENCS_VERIFY,
    ENCS_COMPLETE,
};

enum
{
    ENCM_INITIALIZE,
    ENCM_ACQUIRE,
    ENCM_TRANSFORM,
    ENCM_CUSTOM,
    ENCM_GET_ERROR_STRING,
};

extern int
__enc_decrypt_content(enc_context *enc);

extern enc_provider_proc *
__enc_get_provider(enc_context *enc);

#endif // _ENC_IMPL_H_
