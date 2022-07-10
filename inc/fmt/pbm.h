#ifndef _FMT_PBM_
#define _FMT_PBM_

#include <gfx.h>
#include <pal.h>

// Raw Portable Bitmap (P4)
#define PBM_RAW_MAGIC 0x3450

extern bool
pbm_is_format(hasset asset);

extern bool
pbm_load_bitmap(GFX_BITMAP *bm, hasset asset);

#endif
