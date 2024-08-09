#ifndef _GFX_H_
#define _GFX_H_

#if defined(__linux__) || defined(__MINGW32__)
#define GFX_COLORFUL
#endif

#include <base.h>

typedef struct
{
    int16_t  width;
    int16_t  height;
    uint16_t opl;
    uint8_t  planes;
    uint8_t  bpp;
    void    *bits;
} gfx_bitmap;

typedef struct
{
    int width;
    int height;
} gfx_dimensions;

typedef struct
{
    int left;
    int top;
    int width;
    int height;
} gfx_rect;

typedef enum
{
    GFX_COLOR_BLACK = 0,
    GFX_COLOR_NAVY = 1,
    GFX_COLOR_GREEN = 2,
    GFX_COLOR_TEAL = 3,
    GFX_COLOR_MAROON = 4,
    GFX_COLOR_PURPLE = 5,
    GFX_COLOR_OLIVE = 6,
    GFX_COLOR_SILVER = 7,
    GFX_COLOR_GRAY = 8,
    GFX_COLOR_BLUE = 9,
    GFX_COLOR_LIME = 10,
    GFX_COLOR_CYAN = 11,
    GFX_COLOR_RED = 12,
    GFX_COLOR_FUCHSIA = 13,
    GFX_COLOR_YELLOW = 14,
    GFX_COLOR_WHITE = 15,
    GFX_COLOR_UNKNOWN = -1
} gfx_color;

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

extern bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y);

extern bool
gfx_draw_line(gfx_rect *rect, gfx_color color);

extern bool
gfx_draw_rectangle(gfx_rect *rect, gfx_color color);

extern bool
gfx_fill_rectangle(gfx_rect *rect, gfx_color color);

extern bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y);

extern char
gfx_wctoa(uint16_t wc);

extern char
gfx_wctob(uint16_t wc);

extern gfx_color
gfx_get_color(const char *str);

#endif // _GFX_H_
