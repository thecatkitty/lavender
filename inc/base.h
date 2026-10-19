#ifndef BASE_H
#define BASE_H

/* Generated build configuration */
#include <generated/config.h>

/* Common type headers */
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

/* Polyfill for <stdint.h> <- <inttypes.h> */
#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* Polyfill for <errno.h> */
#ifndef EFTYPE
#define EFTYPE 0x11e
#endif

/* Opaque handle type definition macro */
#define DEFINE_HANDLE(x)                                                       \
    typedef struct                                                             \
    {                                                                          \
        int unused;                                                            \
    } *x

/* Common data size and alignment macros */
#define align(p, a)           (((intptr_t)(p) + (a) - 1) / (a) * (a))
#define lengthof(x)           (sizeof(x) / sizeof((x)[0]))
#define sizeofm(type, member) sizeof(((type *)0)->member)

#endif // BASE_H
