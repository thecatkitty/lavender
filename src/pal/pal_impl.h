#include <pal.h>

typedef struct
{
    off_t inzip;
    int   flags;
    char *data;
} pal_asset;

#define MAX_OPEN_ASSETS 8

extern pal_asset __pal_assets[MAX_OPEN_ASSETS];
