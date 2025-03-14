#ifndef _ARCH_DOS_H_
#define _ARCH_DOS_H_

#include <base.h>

#define interrupt __attribute__((interrupt))

#ifndef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef void interrupt far (*dos_isr)(void);
#pragma GCC diagnostic pop
#endif

extern bool ddcall
dos_is_dosbox(void);

extern bool
dos_is_windows(void);

#ifdef CONFIG_ANDREA
extern uint16_t
dos_load_driver(const char *name);

extern void
dos_unload_driver(uint16_t driver);
#endif

extern void ddcall
dos_beep(uint16_t divisor);

extern void ddcall
dos_silence(void);

#ifdef CONFIG_IA16X
DEFINE_HANDLE(hdosxm);

extern uint16_t
dosxm_init(void);

extern uint16_t
dosxm_getmaxfree(void);

extern hdosxm
dosxm_alloc(uint16_t size_kb);

extern bool
dosxm_free(hdosxm block);

extern bool
dosxm_load(far void *dst, hdosxm src, uint32_t offset, uint32_t length);

extern bool
dosxm_store(hdosxm dst, uint32_t offset, far void *src, uint32_t length);
#endif

#endif // _ARCH_DOS_H_
