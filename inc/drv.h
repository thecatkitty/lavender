#ifndef _DRV_H_
#define _DRV_H_

#include <base.h>

#ifdef LOADABLE
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

#endif // _DRV_H_
