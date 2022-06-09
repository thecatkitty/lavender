#ifndef _PAL_H_
#define _PAL_H_

#include <base.h>
#include <ker.h>

extern void
pal_initialize(ZIP_CDIR_END_HEADER **zip);

extern void
pal_cleanup(void);

#endif // _PAL_H_
