#ifndef _BASE_H_
#define _BASE_H_

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#if defined(_MSC_VER) && (_MSC_VER < 1800)
#define snprintf _snprintf
#endif

#ifndef __cplusplus

#if defined(__ia16__)
#include <libi86/malloc.h>
#include <libi86/string.h>
#else
#include <stdlib.h>
#include <string.h>
#define _ffree   free
#define _fmalloc malloc
#define _fmemcpy memcpy
#define _fmemset memset
#endif

#if defined(_MSC_VER)
#if _MSC_VER >= 1600
#include <intrin.h>
#endif

#define BSWAP16(x) _byteswap_ushort(x)
#define BSWAP32(x) _byteswap_ulong(x)
#define BSWAP64(x) _byteswap_uint64(x)

#define strcasecmp _stricmp
#elif defined(__GNUC__)
#define BSWAP16(x) __builtin_bswap16(x)
#define BSWAP32(x) __builtin_bswap32(x)
#define BSWAP64(x) __builtin_bswap64(x)
#else
extern uint16_t
BSWAP16(uint16_t x);

extern uint32_t
BSWAP32(uint32_t x);
#endif

#ifndef far
#if !defined(EDITING) && defined(__ia16__)
#define far __far
#else
#define far
#endif
#endif

#if defined(__ia16__) && !defined(__IA16_CMODEL_TINY__)
#define ddcall __attribute__((no_assume_ds_data)) far
#else
#define ddcall
#endif

#else

#define far
#define ddcall

#endif

#define DEFINE_HANDLE(x)                                                       \
    typedef struct                                                             \
    {                                                                          \
        int unused;                                                            \
    } *x

#define align(p, a)           (((intptr_t)(p) + (a) - 1) / (a) * (a))
#define lengthof(x)           (sizeof(x) / sizeof((x)[0]))
#define sizeofm(type, member) sizeof(((type *)0)->member)

#if !defined(__cplusplus) && !defined(static_assert)
#define static_assert(expr, msg)
#endif

#ifndef SIZE_MAX
#if defined(_WIN64)
#define SIZE_MAX _UI64_MAX
#else
#define SIZE_MAX UINT_MAX
#endif
#endif

extern uint64_t
rstrtoull(const char *str, int base);

#if !defined(_WIN32) && !__MISC_VISIBLE
extern char *
itoa(int value, char *str, int base);
#endif

extern bool
isdigstr(const char *str);

extern bool
isxdigstr(const char *str);

extern uint8_t
xtob(const char *str);

#ifndef EFTYPE
#define EFTYPE 0x11e
#endif

#endif // _BASE_H_
