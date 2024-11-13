#ifndef _ENC_H_
#define _ENC_H_

#include <base.h>

#include <generated/config.h>

#if defined(CONFIG_ENCRYPTED_CONTENT)
typedef enum
{
    ENC_XOR,
    ENC_DES,
    ENC_TDES,
} enc_cipher;

typedef enum
{
    ENC_KEYSM_RAW,         // not transformed
    ENC_KEYSM_LE32B6D,     // 32 local bits, 6 external decimals
    ENC_KEYSM_PKEY25XOR12, // 25-character product key, XORed 12 characters
} enc_keysm;

typedef enum
{
    ENC_PROVIDER_CALLER, // provided by the caller
    ENC_PROVIDER_PROMPT, // entered by the user
    ENC_PROVIDER_SPLIT,  // part from the caller, completed by the user
    ENC_PROVIDER_DISKID, // part from the diskette, completed by the user
} enc_provider;

typedef struct
{
    const uint8_t *data;
    size_t         data_length;
    const uint8_t *key;
    size_t         key_length;
    const void    *_impl;
    void          *_context;
} enc_stream;

typedef struct
{
    uint32_t local_part;
    uint32_t passcode;
} enc_split_data;

typedef struct
{
    enc_split_data split;
    char           dsn[16];
} enc_diskid_data;

typedef struct
{
    enc_cipher  cipher;
    int         provider;
    const void *parameter;
    uint8_t    *content;
    int         size;
    uint32_t    crc32;

    int        state;
    enc_stream stream;
    union {
        uint64_t qw;
        uint8_t  b[16];
    } key;
    char buffer[34];
    union {
        enc_split_data  split;
        enc_diskid_data diskid;
    } data;
} enc_context;

#define ENC_KEYSRC(p, sm) (((sm) << 8) | (p))

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

extern int
enc_decode_key(const void *src, void *dst, enc_keysm sm);

extern bool
enc_validate_key_format(const char *key, enc_keysm sm);

extern int
enc_access_content(enc_context *enc,
                   enc_cipher   cipher,
                   int          provider,
                   const void  *parameter,
                   uint8_t     *content,
                   size_t       length,
                   uint32_t     crc32);

extern int
enc_handle(enc_context *enc);
#endif // CONFIG_ENCRYPTED_CONTENT

#endif // _ENC_H_
