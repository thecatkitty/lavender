#ifndef _GFX_H_
#define _GFX_H_

#include <gfx/types.h>

#define GFX_COLUMNS 80
#define GFX_LINES   25

extern bool
gfx_initialize(void);

extern void
gfx_cleanup(void);

// Get width and height of the screen area in pixels
extern void
gfx_get_screen_dimensions(gfx_dimensions *dim);

// Get width and height of a glyph in pixels
extern void
gfx_get_glyph_dimensions(gfx_dimensions *dim);

// Get pixel aspect ratio
// Returns pixel aspect ratio (PAR = 64 / value), default value on error
extern uint16_t
gfx_get_pixel_aspect(void);

#if defined(CONFIG_HAVE_GFX_SCALING)
// Get display scaling factor
extern float
gfx_get_scale(void);

// Change display scaling factor by direction (-1 for smaller, +1 for larger)
extern bool
gfx_step_scale(int direction);
#endif

#if defined(CONFIG_HAVE_GFX_CHARSET)
extern bool
gfx_get_font_data(gfx_glyph_data *data);
#endif

extern unsigned
gfx_get_color_depth(void);

extern bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y);

extern bool
gfx_draw_line(const gfx_rect *rect, gfx_color color);

extern bool
gfx_draw_rectangle(const gfx_rect *rect, gfx_color color);

extern bool
gfx_fill_rectangle(const gfx_rect *rect, gfx_color color);

extern bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y);

extern gfx_color
gfx_get_color(const char *str);

extern bool
gfx_set_title(const char *title);

#endif // _GFX_H_
