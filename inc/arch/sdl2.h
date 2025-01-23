#ifndef _ARCH_SDL2_H_
#define _ARCH_SDL2_H_

#include <base.h>

extern bool
sdl2_initialize(void);

extern void
sdl2_cleanup(void);

extern const char *
sdl2_get_font(void);

extern bool
sdl2_set_scale(int scale);

#endif // _ARCH_SDL2_H_
