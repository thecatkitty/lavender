#ifndef COMPAT_BSWAP_H
#define COMPAT_BSWAP_H

#if defined(_MSC_VER)
#if _MSC_VER >= 1600
#include <intrin.h>
#endif

#define BSWAP16(x) _byteswap_ushort(x)
#define BSWAP32(x) _byteswap_ulong(x)
#define BSWAP64(x) _byteswap_uint64(x)
#elif defined(__GNUC__)
#define BSWAP16(x) __builtin_bswap16(x)
#define BSWAP32(x) __builtin_bswap32(x)
#define BSWAP64(x) __builtin_bswap64(x)
#else
#include <stdint.h>

extern uint16_t
BSWAP16(uint16_t x);

extern uint32_t
BSWAP32(uint32_t x);
#endif

#endif
