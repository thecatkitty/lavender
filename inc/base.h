#ifndef _BASE_H_
#define _BASE_H_

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

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

typedef union {
    uint64_t qw;
    uint32_t dw[sizeof(uint64_t) / sizeof(uint32_t)];
    uint16_t w[sizeof(uint64_t) / sizeof(uint16_t)];
    uint8_t  b[sizeof(uint64_t) / sizeof(uint8_t)];
} uquad;

extern uint64_t
rstrtoull(const char *restrict str, int base);

#if !__MISC_VISIBLE
extern char *
itoa(int value, char *str, int base);
#endif

#ifndef EFTYPE
#define EFTYPE 0x11e
#endif

#endif // _BASE_H_
