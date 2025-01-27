#ifndef _PAL_IMPL_H_
#define _PAL_IMPL_H_

#include <fmt/zip.h>
#include <pal.h>

typedef struct
{
    zip_item inzip;
    int      flags;
    char    *data;
} pal_asset;

#ifndef O_ACCMODE
#define O_ACCMODE (_O_RDONLY | _O_WRONLY | _O_RDWR)
#endif

#define MAX_OPEN_ASSETS 8

extern pal_asset pal_assets[MAX_OPEN_ASSETS];

#endif // _PAL_IMPL_H_
