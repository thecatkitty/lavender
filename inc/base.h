#ifndef _BASE_H_
#define _BASE_H_

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#ifndef EDITING
#define far __far
#else
#define far
#endif

#define DEFINE_HANDLE(x)                                                       \
    typedef struct                                                             \
    {                                                                          \
    } *x

#define sizeofm(type, member) sizeof(((type *)0)->member)

typedef union {
    uint64_t qw;
    uint32_t dw[sizeof(uint64_t) / sizeof(uint32_t)];
    uint16_t w[sizeof(uint64_t) / sizeof(uint16_t)];
    uint8_t  b[sizeof(uint64_t) / sizeof(uint8_t)];
} uquad;

extern uint64_t
rstrtoull(const char *restrict str, int base);

#ifndef EFTYPE
#define EFTYPE 0x11e
#endif

#endif // _BASE_H_
