#ifndef _VID_H_
#define _VID_H_

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <gfx.h>

#define VID_MODE_CGA_HIMONO 6 // 640x200x1

#define VID_CGA_HIMONO_WIDTH  640
#define VID_CGA_HIMONO_HEIGHT 200
#define VID_CGA_HIMONO_LINE   (VID_CGA_HIMONO_WIDTH / 8)

#pragma pack(push, 1)
typedef struct
{
    uint16_t CodePoint;
    char *   Overlay;
    char *   Transformation;
    char     Base;
} VID_CHARACTER_DESCRIPTOR;
#pragma pack(pop)

// Set video mode
// Returns previous video mode
extern uint16_t
VidSetMode(uint16_t mode);

// Get pixel aspect ratio
// Returns pixel aspect ratio (PAR = 64 / value), default value on error
extern uint16_t
VidGetPixelAspectRatio(void);

extern int
VidDrawBitmap(GFX_BITMAP *bm, uint16_t x, uint16_t y);

extern int
VidDrawRectangle(GFX_DIMENSIONS *rect, uint16_t x, uint16_t y, GFX_COLOR color);

extern int
VidDrawText(const char *str, uint16_t x, uint16_t y);

extern void
VidLoadFont(void);

extern void
VidUnloadFont(void);

extern char
VidConvertToLocal(uint16_t wc);

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_VID_UNSUPPORTED ERR_CODE(ERR_FACILITY_VID, 0)
#define ERR_VID_FAILED      ERR_CODE(ERR_FACILITY_VID, 1)
#define ERR_VID_FORMAT      ERR_CODE(ERR_FACILITY_VID, 2)

#define VID_GXF_CMD_GROW   0
#define VID_GXF_CMD_SELECT 1
#define VID_GXF_CMD_MOVE   2
#define VID_GXF_CMD_CLEAR  3

#ifdef __ASSEMBLER__

#define GXF_GROW(n)   ((VID_GXF_CMD_GROW << 4) | (n & 0xF))
#define GXF_SELECT(n) ((VID_GXF_CMD_SELECT << 4) | (n & 0xF))
#define GXF_MOVE(n)   ((VID_GXF_CMD_MOVE << 4) | (n & 0xF))
#define GXF_CLEAR(n)  ((VID_GXF_CMD_CLEAR << 4) | (n & 0xF))
#define GXF_END       GXF_GROW(0)

#endif // __ASSEMBLER__

#endif // _VID_H_
