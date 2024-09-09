#ifndef _GLYPH_H_
#define _GLYPH_H_

#include <drv.h>
#include <gfx.h>

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

extern const gfx_glyph DRV_RDAT __gfx_font_8x8[];
extern const uint8_t DRV_RDAT   __gfx_overlays[][1 + GFX_MAX_OVERLAY_SIZE];
extern const uint8_t DRV_RDAT   __gfx_xforms[][GFX_MAX_TRANSFORMATION_SIZE];

#endif // _GLYPH_H_
