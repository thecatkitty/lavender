#ifndef _CRG_H_
#define _CRG_H_

#ifndef __ASSEMBLER__

#include <base.h>

typedef enum
{
    CRG_XOR
} crg_cipher;

typedef struct
{
    const uint8_t *data;
    size_t         data_length;
    const uint8_t *key;
    size_t         key_length;
    const void    *_impl;
} crg_stream;

typedef bool (*CRG_KEY_VALIDATOR)(const uint8_t *key,
                                  int            keyLength,
                                  void          *context);

extern bool
crg_prepare(crg_stream    *stream,
            crg_cipher     cipher,
            const uint8_t *data,
            size_t         data_length,
            const uint8_t *key,
            size_t         key_length);

extern uint8_t
crg_at(crg_stream *stream, size_t i);

extern bool
crg_decrypt(crg_stream *stream, uint8_t *dst);

extern bool
crg_validate(crg_stream *stream, uint32_t crc);

extern bool
CrgPromptKey(uint8_t *         key,
             int               keyLength,
             int               base,
             CRG_KEY_VALIDATOR validate,
             void *            context);

extern uint64_t
CrgDecodeSplitKey(uint32_t longPart, uint32_t shortPart);

#endif // __ASSEMBLER__

#endif // _CRG_H_
