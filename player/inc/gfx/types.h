#ifndef GFX_TYPES_H
#define GFX_TYPES_H

#include <base.h>
#include <compat/far.h>

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

#pragma pack(push, 1)
typedef struct
{
    uint16_t codepoint;
    char     base;
    uint8_t  overlay;
    uint8_t  transformation;
} gfx_glyph;
#pragma pack(pop)

typedef far const gfx_glyph *gfx_glyph_data;

#endif // GFX_TYPES_H
