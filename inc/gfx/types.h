#ifndef GFX_TYPES_H
#define GFX_TYPES_H

#include <base.h>

typedef struct
{
    int16_t  width;
    int16_t  height;
    uint16_t opl;
    uint8_t  bpp;
    void    *bits;
    off_t    offset;
    int16_t  chunk_top;
    int16_t  chunk_height;
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

#endif // GFX_TYPES_H
