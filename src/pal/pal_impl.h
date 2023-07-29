#include <pal.h>

typedef struct
{
    off_t inzip;
    int   flags;
    char *data;
} pal_asset;

#define MAX_OPEN_ASSETS 8

extern pal_asset __pal_assets[MAX_OPEN_ASSETS];

#ifdef __linux__

#include <stdio.h>

extern void
__pal_log_time(void);

#define LOG(fmt, ...)                                                          \
    {                                                                          \
        __pal_log_time();                                                      \
        fprintf(stderr, "%s: ", __FUNCTION__);                                 \
        fprintf(stderr, fmt "\n", ##__VA_ARGS__);                              \
    }

#else

#define LOG(fmt, ...)

#endif
