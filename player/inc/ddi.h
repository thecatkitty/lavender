#ifndef DDI_H
#define DDI_H

#include <base.h>
#include <compat/far.h>

#if defined(MODULE)
#if !defined(CONFIG_DOS)
#error "We're not there yet"
#endif

#include <andrea.h>

#define DRV_DATA ANDREA_MODDATA
#define DRV_RDAT __attribute__((section(".text.rdat"))) far
#define DRV_INIT(name)                                                         \
    extern int ddcall drv_init(void);                                          \
    ANDREA_EXPORT(drv_init);                                                   \
    int ddcall drv_init
#else
#define DRV_DATA
#define DRV_RDAT
#define DRV_INIT(name) int ddcall __##name##_init
#endif

#define DEV_MAX_NAME        9
#define DEV_MAX_DESCRIPTION 32

typedef struct
{
    char name[DEV_MAX_NAME];
    char description[DEV_MAX_DESCRIPTION];

    far void *ops;
    far void *data;
} device;

#endif // DDI_H
