#ifndef _GFX_H_
#define _GFX_H_

#ifndef __ASSEMBLER__

#include <stdint.h>

typedef struct
{
    uint16_t Width;
    uint16_t Height;
    uint16_t WidthBytes;
    uint8_t  Planes;
    uint8_t  BitsPerPixel;
    void *   Bits;
} GFX_BITMAP;

// Load bitmap picture
// Return negative on error
extern int
GfxLoadBitmap(const void *data, GFX_BITMAP *bm);

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_GFX_FORMAT         ERR_CODE(ERR_FACILITY_GFX, 0)
#define ERR_GFX_MALFORMED_FILE ERR_CODE(ERR_FACILITY_GFX, 1)

#endif // _GFX_H_
