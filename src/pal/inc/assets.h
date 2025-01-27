#ifndef _PAL_IMPL_H_
#define _PAL_IMPL_H_

#include <fmt/zip.h>
#include <pal.h>

typedef struct
{
    zip_item inzip;
    int      flags;
    int      opts;

    union {
        char      *data;
        zip_cached handle;
    };
} pal_asset;

#ifndef O_ACCMODE
#define O_ACCMODE (_O_RDONLY | _O_WRONLY | _O_RDWR)
#endif

#define PALOPT_LOCAL 0x0001
#define PALOPT_CACHE 0x0002
#define PALOPT_WHERE 0x0003

#define MAX_OPEN_ASSETS 8

extern pal_asset pal_assets[MAX_OPEN_ASSETS];

#endif // _PAL_IMPL_H_
