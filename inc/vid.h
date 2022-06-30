#ifndef _VID_H_
#define _VID_H_

#ifndef __ASSEMBLER__

#include <gfx.h>

// Set video mode
// Returns previous video mode
extern uint16_t
VidSetMode(uint16_t mode);

// Get width and height of the screen area in pixels
extern void
VidGetScreenDimensions(GFX_DIMENSIONS *dim);

// Get pixel aspect ratio
// Returns pixel aspect ratio (PAR = 64 / value), default value on error
extern uint16_t
VidGetPixelAspectRatio(void);

extern int
VidDrawBitmap(GFX_BITMAP *bm, uint16_t x, uint16_t y);

extern int
VidDrawLine(GFX_DIMENSIONS *dim, uint16_t x, uint16_t y, GFX_COLOR color);

extern int
VidDrawRectangle(GFX_DIMENSIONS *rect, uint16_t x, uint16_t y, GFX_COLOR color);

extern int
VidFillRectangle(GFX_DIMENSIONS *rect, uint16_t x, uint16_t y, GFX_COLOR color);

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

#define ERR_VID_FORMAT      ERR_CODE(ERR_FACILITY_VID, 2)

#endif // _VID_H_
