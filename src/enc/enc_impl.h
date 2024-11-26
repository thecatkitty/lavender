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
extern enc_provider_proc __enc_remote_proc;

#define CONTINUE 1

enum
{
    ENCS_INITIALIZE,
    ENCS_READ,
    ENCS_VERIFY,
    ENCS_COMPLETE,
};

enum
{
    ENCM_INITIALIZE,
    ENCM_TRANSFORM,
    ENCM_GET_ERROR_STRING,
};

// Decrypt content described by the encryption context
// @returns Zero on success, positive error message ID, or negative on other error
extern int
__enc_decrypt_content(enc_context *enc);

extern void
__enc_des_expand56(uint64_t src, uint8_t *dst);

#endif // _ENC_IMPL_H_
