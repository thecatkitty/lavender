#ifndef _CRG_H_
#define _CRG_H_

#include <base.h>

typedef enum
{
    CRG_XOR,
    CRG_DES
} crg_cipher;

typedef enum
{
    CRG_KEYSM_LE32B6D, // 32 local bits, 6 external decimals
} crg_keysm;

typedef struct
{
    const uint8_t *data;
    size_t         data_length;
    const uint8_t *key;
    size_t         key_length;
    const void    *_impl;
    void          *_context;
} crg_stream;

extern bool
crg_prepare(crg_stream    *stream,
            crg_cipher     cipher,
            const uint8_t *data,
            size_t         data_length,
            const uint8_t *key,
            size_t         key_length);

extern bool
crg_free(crg_stream *stream);

extern uint8_t
crg_at(crg_stream *stream, size_t i);

extern bool
crg_decrypt(crg_stream *stream, uint8_t *dst);

extern bool
crg_validate(crg_stream *stream, uint32_t crc);

extern uint64_t
crg_decode_key(const void *src, crg_keysm sm);

#endif // _CRG_H_
