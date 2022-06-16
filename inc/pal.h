#ifndef _PAL_H_
#define _PAL_H_

#include <base.h>
#include <ker.h>

extern void
pal_initialize(ZIP_CDIR_END_HEADER **zip);

extern void
pal_cleanup(void);

extern void
pal_sleep(unsigned ms);

extern void
pal_beep(uint16_t divisor);

#endif // _PAL_H_
