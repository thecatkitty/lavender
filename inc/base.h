#ifndef _BASE_H_
#define _BASE_H_

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
    } * x

extern uint64_t
rstrtoull(const char *restrict str, int base);

#endif // _BASE_H_
