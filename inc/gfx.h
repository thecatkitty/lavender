#ifndef _GFX_H_
#define _GFX_H_

#ifndef __ASSEMBLER__

#include <base.h>

typedef struct
{
    uint16_t Width;
    uint16_t Height;
    uint16_t WidthBytes;
    uint8_t  Planes;
    uint8_t  BitsPerPixel;
    void *   Bits;
} GFX_BITMAP;

typedef struct
{
    int Width;
    int Height;
} GFX_DIMENSIONS;

typedef enum
{
    GFX_COLOR_BLACK,
    GFX_COLOR_WHITE,
    GFX_COLOR_GRAY50
} GFX_COLOR;

// Load bitmap picture
// Return negative on error
int
PbmLoadBitmap(const char *data, GFX_BITMAP *bm);

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_GFX_MALFORMED_FILE ERR_CODE(ERR_FACILITY_GFX, 1)

#endif // _GFX_H_
