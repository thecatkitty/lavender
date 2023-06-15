#ifndef _PAL_H_
#define _PAL_H_

#include <fcntl.h>

#include <base.h>

DEFINE_HANDLE(hasset);

#define PAL_MOUSE_LBUTTON 0x0001
#define PAL_MOUSE_RBUTTON 0x0002

extern void
pal_initialize(int argc, char *argv[]);

extern void
pal_cleanup(void);

extern uint32_t
pal_get_counter(void);

extern uint32_t
pal_get_ticks(unsigned ms);

extern void
pal_sleep(unsigned ms);

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

extern uint16_t
pal_get_keystroke(void);

extern void
pal_enable_mouse(void);

extern void
pal_disable_mouse(void);

extern uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y);

extern int
pal_load_string(unsigned id, char *buffer, int max_length);

#endif // _PAL_H_
