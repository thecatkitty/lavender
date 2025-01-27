#include <fmt/bmp.h>
#include <pal.h>

#define XRGB_RED_MASK   0x00FF0000UL
#define XRGB_GREEN_MASK 0x0000FF00UL
#define XRGB_BLUE_MASK  0x000000FFUL

bool
bmp_load_bitmap(gfx_bitmap *bm, hasset asset)
{
    const bmp_file_header *fh;
    const bmp_info_header *ih;
    char                  *data;

    LOG("entry, bm: %p, asset: %p", (void *)bm, (void *)asset);

    data = pal_load_asset(asset);
    fh = (const bmp_file_header *)data;
    if (BMP_MAGIC != fh->type)
    {
        LOG("exit, not a BMP!");
        errno = EFTYPE;
        return false;
    }

    bm->bits = data + fh->off_bits;

    ih = (const bmp_info_header *)(data + sizeof(bmp_file_header));
    switch (ih->size)
    {
    case sizeof(bmp_info_header):
        LOG("found BITMAPINFOHEADER");
        break;
    case sizeof(bmp_v5_header):
        LOG("found BITMAPV5HEADER");
        break;
    default:
        LOG("exit, unknown bitmap header of size %u!", ih->size);
        errno = EFTYPE;
        return false;
    }

    bm->width = ih->width;
    bm->height = -ih->height;
    bm->planes = ih->planes;
    bm->bpp = ih->bit_count;
    LOG("%dx%d, %u planes, %u bpp", bm->width, bm->height, bm->planes, bm->bpp);

    if ((1 != bm->bpp) && (4 != bm->bpp) && (32 != bm->bpp))
    {
        LOG("exit, %d bit depth not supported!", bm->bpp);
        errno = EFTYPE;
        return false;
    }

    if (BMP_COMPRESSION_BITFIELDS == ih->compression)
    {
        const bmp_v5_header *v5 = (const bmp_v5_header *)ih;
        if ((XRGB_RED_MASK != v5->red_mask) ||
            (XRGB_GREEN_MASK != v5->green_mask) ||
            (XRGB_BLUE_MASK != v5->blue_mask))
        {
            LOG("exit, %08x %08x %08x bitmask not supported!", v5->red_mask,
                v5->green_mask, v5->blue_mask);
            errno = EFTYPE;
            return false;
        }
    }
    else if (BMP_COMPRESSION_RGB != ih->compression)
    {
        LOG("exit, compression %u not supported!", ih->compression);
        errno = EFTYPE;
        return false;
    }

    bm->opl = (((bm->width * bm->bpp) + 31) & ~31) >> 3;
    return true;
}
