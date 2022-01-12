#ifndef _GFX_H_
#define _GFX_H_

#include <stdint.h>

#include <err.h>

#define ERR_GFX_FORMAT                  ERR_CODE(ERR_FACILITY_GFX, 0)
#define ERR_GFX_MALFORMED_FILE          ERR_CODE(ERR_FACILITY_GFX, 1)

typedef struct {
  uint16_t Width;
  uint16_t Height;
  uint16_t WidthBytes;
  uint8_t  Planes;
  uint8_t  BitsPerPixel;
  void     *Bits;
} GFX_BITMAP;

// Load bitmap picture
// Return negative on error
extern int GfxLoadBitmap(
    const void *data,
    GFX_BITMAP *bm);

#endif // _GFX_H_
