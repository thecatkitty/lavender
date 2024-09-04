#ifndef _DEV_H_
#define _DEV_H_

#include <base.h>

#define DEV_MAX_NAME        9
#define DEV_MAX_DESCRIPTION 32

typedef struct
{
    char name[DEV_MAX_NAME];
    char description[DEV_MAX_DESCRIPTION];

    far void *ops;
    far void *data;
} device;

#endif // _DEV_H_
