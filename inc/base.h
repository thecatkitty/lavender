#ifndef _BASE_H_
#define _BASE_H_

#include <limits.h>

#include <stdbool.h>
#include <stdint.h>

#ifndef EDITING
#define far __far
#else
#define far
#endif

#define EXIT_ERRNO 512

#define DEFINE_HANDLE(x)                                                       \
    typedef struct                                                             \
    {                                                                          \
    } * x

#endif // _BASE_H_
