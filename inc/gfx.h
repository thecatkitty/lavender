#ifndef _GFX_H_
#define _GFX_H_

#include <base.h>

typedef struct
{
    uint16_t width;
    uint16_t height;
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

typedef enum
{
    GFX_COLOR_BLACK,
    GFX_COLOR_WHITE,
    GFX_COLOR_GRAY
} gfx_color;

extern bool
gfx_initialize(void);

extern void
gfx_cleanup(void);

// Get width and height of the screen area in pixels
extern void
gfx_get_screen_dimensions(gfx_dimensions *dim);

// Get pixel aspect ratio
// Returns pixel aspect ratio (PAR = 64 / value), default value on error
extern uint16_t
gfx_get_pixel_aspect(void);

extern bool
gfx_draw_bitmap(gfx_bitmap *bm, uint16_t x, uint16_t y);

extern bool
gfx_draw_line(gfx_dimensions *dim, uint16_t x, uint16_t y, gfx_color color);

extern bool
gfx_draw_rectangle(gfx_dimensions *rect,
                   uint16_t        x,
                   uint16_t        y,
                   gfx_color       color);

extern bool
gfx_fill_rectangle(gfx_dimensions *rect,
                   uint16_t        x,
                   uint16_t        y,
                   gfx_color       color);

extern bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y);

extern char
gfx_wctob(uint16_t wc);

#endif // _GFX_H_
