#ifndef _PLATFORM_DOSPC_H_
#define _PLATFORM_DOSPC_H_

#include <base.h>

#define interrupt __attribute__((interrupt))

#ifndef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
typedef void interrupt far (*dospc_isr)(void);
#pragma GCC diagnostic pop
#endif

extern bool
dospc_is_dosbox(void);

#ifdef CONFIG_ANDREA
extern uint16_t
dospc_load_driver(const char *name);

extern void
dospc_unload_driver(uint16_t driver);
#endif

extern void ddcall
dospc_beep(uint16_t divisor);

extern void ddcall
dospc_silence(void);

#endif // _PLATFORM_DOSPC_H_
