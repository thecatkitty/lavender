#ifndef _BASE_H_
#define _BASE_H_

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#if !defined(_MSC_VER) || (_MSC_VER >= 1800)
#include <stdbool.h>
#else
typedef int bool;
#define false 0
#define true 1
#endif

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
#include <intrin.h>

#define __builtin_bswap16(x) _byteswap_ushort(x)
#define __builtin_bswap32(x) _byteswap_ulong(x)
#define __builtin_bswap64(x) _byteswap_uint64(x)

#define strcasecmp _stricmp
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

#define DEFINE_HANDLE(x)                                                       \
    typedef struct                                                             \
    {                                                                          \
        int unused;                                                            \
    } *x

#define align(p, a)           (((intptr_t)(p) + (a)-1) / (a) * (a))
#define lengthof(x)           (sizeof(x) / sizeof((x)[0]))
#define sizeofm(type, member) sizeof(((type *)0)->member)

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
