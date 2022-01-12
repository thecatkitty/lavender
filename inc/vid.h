#ifndef _VID_H_
#define _VID_H_

#include <stdint.h>

#include <err.h>
#include <gfx.h>

#define VID_CGA_HIMONO_WIDTH  640
#define VID_CGA_HIMONO_HEIGHT 200
#define VID_CGA_HIMONO_LINE   (VID_CGA_HIMONO_WIDTH / 8)

#define ERR_VID_UNSUPPORTED ERR_CODE(ERR_FACILITY_VID, 0)
#define ERR_VID_FAILED      ERR_CODE(ERR_FACILITY_VID, 1)
#define ERR_VID_FORMAT      ERR_CODE(ERR_FACILITY_VID, 2)

#pragma pack(push, 1)
typedef struct
{
    uint16_t CodePoint;
    char *   Overlay;
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
VidDrawText(const char *str, uint16_t x, uint16_t y);

extern void
VidLoadFont(void);

#endif // _VID_H_
