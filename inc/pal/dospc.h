#ifndef _PAL_DOSPC_H_
#define _PAL_DOSPC_H_

#include <base.h>

#define interrupt __attribute__((interrupt))

typedef void interrupt far (*ISR)(void);

extern bool
dospc_is_dosbox(void);

#endif // _PAL_DOSPC_H_
