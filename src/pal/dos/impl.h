#ifndef _PAL_DOS_IMPL_H_
#define _PAL_DOS_IMPL_H_

#include <gfx.h>

extern gfx_glyph_data dos_font;
extern gfx_dimensions dos_cell;
extern bool           dos_mouse;

extern void
dos_initialize_cache(void);

extern void
dos_cleanup_cache(void);

#endif // _PAL_DOS_IMPL_H_
