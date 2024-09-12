#include <gfx.h>

bool
gfx_initialize(void)
{
    return true;
}

void
gfx_get_screen_dimensions(gfx_dimensions *dim)
{
    dim->width = 640;
    dim->height = 480;
}

void
gfx_get_glyph_dimensions(gfx_dimensions *dim)
{
    dim->width = 8;
    dim->height = 16;
}

uint16_t
gfx_get_pixel_aspect(void)
{
    return 64 * 1;
}

unsigned
gfx_get_color_depth(void)
{
    return 24;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    return true;
}

bool
gfx_draw_line(gfx_rect *rect, gfx_color color)
{
    return true;
}

bool
gfx_draw_rectangle(gfx_rect *rect, gfx_color color)
{
    return true;
}

bool
gfx_fill_rectangle(gfx_rect *rect, gfx_color color)
{
    return true;
}

bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y)
{
    return true;
}

void
gfx_cleanup(void)
{
}
