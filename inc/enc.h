#ifndef _ENC_H_
#define _ENC_H_

#include <base.h>

#include <generated/config.h>

#if defined(CONFIG_ENCRYPTED_CONTENT)
typedef enum
{
    ENC_XOR,
    ENC_DES
} enc_cipher;

typedef enum
{
    ENC_KEYSM_LE32B6D,     // 32 local bits, 6 external decimals
    ENC_KEYSM_PKEY25XOR12, // 25-character product key, XORed 12 characters
} enc_keysm;

typedef struct
{
    const uint8_t *data;
    size_t         data_length;
    const uint8_t *key;
    size_t         key_length;
    const void    *_impl;
    void          *_context;
} enc_stream;

extern bool
enc_prepare(enc_stream    *stream,
            enc_cipher     cipher,
            const uint8_t *data,
            size_t         data_length,
            const uint8_t *key,
            size_t         key_length);

extern bool
enc_free(enc_stream *stream);

extern uint8_t
enc_at(enc_stream *stream, size_t i);

extern bool
enc_decrypt(enc_stream *stream, uint8_t *dst);

extern bool
enc_verify(enc_stream *stream, uint32_t crc);

extern uint64_t
enc_decode_key(const void *src, enc_keysm sm);

extern bool
enc_validate_key_format(const char *key, enc_keysm sm);
#endif // CONFIG_ENCRYPTED_CONTENT

#endif // _ENC_H_
