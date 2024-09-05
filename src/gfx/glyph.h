#ifndef _GLYPH_H_
#define _GLYPH_H_

#include <stdint.h>

#pragma pack(push, 1)
typedef struct
{
    uint16_t codepoint;
    char     base;
    uint8_t  overlay;
    uint8_t  transformation;
} gfx_glyph;
#pragma pack(pop)

#define GFX_MAX_OVERLAY_SIZE        3
#define GFX_MAX_TRANSFORMATION_SIZE 7

#define GXF_CMD_GROW   0
#define GXF_CMD_SELECT 1
#define GXF_CMD_MOVE   2
#define GXF_CMD_CLEAR  3

#define GXF_GROW(n)   ((GXF_CMD_GROW << 4) | (n & 0xF))
#define GXF_SELECT(n) ((GXF_CMD_SELECT << 4) | (n & 0xF))
#define GXF_MOVE(n)   ((GXF_CMD_MOVE << 4) | (n & 0xF))
#define GXF_CLEAR(n)  ((GXF_CMD_CLEAR << 4) | (n & 0xF))
#define GXF_END       GXF_GROW(0)

extern const gfx_glyph __gfx_font_8x8[];
extern const uint8_t   __gfx_overlays[][1 + GFX_MAX_OVERLAY_SIZE];
extern const uint8_t   __gfx_transformations[][GFX_MAX_TRANSFORMATION_SIZE];

#endif // _GLYPH_H_
