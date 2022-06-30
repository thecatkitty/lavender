#ifndef _PAL_DOSPC_H_
#define _PAL_DOSPC_H_

#include <base.h>

#define interrupt __attribute__((interrupt))

typedef void interrupt far (*dospc_isr)(void);

extern bool
dospc_is_dosbox(void);

extern void
dospc_beep(uint16_t divisor);

extern void
dospc_silence(void);

#endif // _PAL_DOSPC_H_
