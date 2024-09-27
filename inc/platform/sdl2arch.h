#ifndef _PLATFORM_SDL2ARCH_H_
#define _PLATFORM_SDL2ARCH_H_

#include <base.h>

extern bool
sdl2arch_initialize(void);

extern void
sdl2arch_cleanup(void);

extern const char *
sdl2arch_get_font(void);

extern void
sdl2arch_set_window_title(const char *title);

extern bool
sdl2arch_set_scale(int scale);

#endif // _PLATFORM_SDL2ARCH_H_
