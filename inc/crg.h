#ifndef _CRG_H_
#define _CRG_H_

#ifndef __ASSEMBLER__

#include <base.h>

extern bool
CrgIsXorKeyValid(const void *   data,
                 int            dataLength,
                 const uint8_t *key,
                 int            keyLength,
                 uint32_t       crc);

extern void
CrgXor(const void *   src,
       void *         dst,
       int            dataLength,
       const uint8_t *key,
       int            keyLength);

extern bool
CrgPromptKey(uint8_t *key, int keyLength, int base);

#endif // __ASSEMBLER__

#endif // _CRG_H_
