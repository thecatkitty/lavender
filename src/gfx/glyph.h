#ifndef _GLYPH_H_
#define _GLYPH_H_

#include <stdint.h>

#pragma pack(push, 1)
typedef struct
{
    uint16_t       codepoint;
    const uint8_t *overlay;
    const uint8_t *transformation;
    char           base;
} vid_glyph;
#pragma pack(pop)

#define GXF_CMD_GROW   0
#define GXF_CMD_SELECT 1
#define GXF_CMD_MOVE   2
#define GXF_CMD_CLEAR  3

#define GXF_GROW(n)   ((GXF_CMD_GROW << 4) | (n & 0xF))
#define GXF_SELECT(n) ((GXF_CMD_SELECT << 4) | (n & 0xF))
#define GXF_MOVE(n)   ((GXF_CMD_MOVE << 4) | (n & 0xF))
#define GXF_CLEAR(n)  ((GXF_CMD_CLEAR << 4) | (n & 0xF))
#define GXF_END       GXF_GROW(0)

#endif // _GLYPH_H_
