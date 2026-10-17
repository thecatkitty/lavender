#ifndef _BASE_H_
#define _BASE_H_

#include <generated/config.h>

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

#ifndef __cplusplus

#if defined(EDITING)

#ifndef far
#define far
#endif

#endif

#ifndef far
#if defined(__ia16__)
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
