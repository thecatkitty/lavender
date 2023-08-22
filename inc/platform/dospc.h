#ifndef _PLATFORM_DOSPC_H_
#define _PLATFORM_DOSPC_H_

#include <base.h>

#define interrupt __attribute__((interrupt))

#ifndef __linux__
typedef void interrupt far (*dospc_isr)(void);
#endif

extern bool
dospc_is_dosbox(void);

extern void
dospc_beep(uint16_t divisor);

extern void
dospc_silence(void);

#endif // _PLATFORM_DOSPC_H_
