#ifndef _PAL_H_
#define _PAL_H_

#include <fcntl.h>

#include <base.h>
#include <ker.h>

typedef struct
{
} * hasset;

extern void
pal_initialize(void);

extern void
pal_cleanup(int status);

extern void
pal_sleep(unsigned ms);

extern void
pal_beep(uint16_t divisor);

extern hasset
pal_open_asset(const char *name, int flags);

extern bool
pal_close_asset(hasset asset);

extern char *
pal_get_asset_data(hasset asset);

extern int
pal_get_asset_size(hasset asset);

extern const char *
pal_get_version_string(void);

extern uint32_t
pal_get_medium_id(const char *tag);

#endif // _PAL_H_
