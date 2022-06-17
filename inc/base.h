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

#define container_of(ptr, type, member)                                        \
    ({                                                                         \
        const typeof(((type *)0)->member) *_pmember = (ptr);                   \
        (type *)((char *)_pmember - offsetof(type, member));                   \
    })

#endif // _BASE_H_
