#include <stdlib.h>

#include <fmt/bmp.h>
#include <pal.h>

#define XRGB_RED_MASK   0x00FF0000UL
#define XRGB_GREEN_MASK 0x0000FF00UL
#define XRGB_BLUE_MASK  0x000000FFUL

#if SIZE_MAX <= UINT16_MAX
#define MAX_CHUNK 5120 // 640x2 @ 32bpp, 640x16 @ 4bpp, 640x48 @ 1bpp
#else
#define MAX_CHUNK UINT16_MAX
#endif

static bool
deallocate(gfx_bitmap *bm, int reason)
{
    free(bm->bits);
    bm->bits = NULL;
    errno = reason ? reason : errno;
    return false;
}

static bool
prepare(gfx_bitmap *bm, hasset asset)
{
    const bmp_file_header *fh;
    const bmp_info_header *ih;

    LOG("entry, bm: %p, asset: %p", (void *)bm, (void *)asset);

    bm->bits = malloc(MAX_CHUNK);
    if (NULL == bm->bits)
    {
        return false;
    }

    if (!pal_read_asset(asset, bm->bits, 0,
                        sizeof(bmp_file_header) + sizeof(bmp_v5_header)))
    {
        return deallocate(bm, 0);
    }

    fh = (const bmp_file_header *)bm->bits;
    if (BMP_MAGIC != fh->type)
    {
        LOG("exit, not a BMP!");
        return deallocate(bm, EFTYPE);
    }

    ih = (const bmp_info_header *)((char *)bm->bits + sizeof(bmp_file_header));
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
        return deallocate(bm, EFTYPE);
    }

    bm->width = ih->width;
    bm->height = -ih->height;
    bm->bpp = ih->bit_count;
    LOG("%dx%d, %u bpp", bm->width, bm->height, bm->bpp);

    if (1 != ih->planes)
    {
        LOG("exit, %d planes not supported!", ih->planes);
        return deallocate(bm, EFTYPE);
    }

    if ((1 != bm->bpp) && (4 != bm->bpp) && (32 != bm->bpp))
    {
        LOG("exit, %d bit depth not supported!", bm->bpp);
        return deallocate(bm, EFTYPE);
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
            return deallocate(bm, EFTYPE);
        }
    }
    else if (BMP_COMPRESSION_RGB != ih->compression)
    {
        LOG("exit, compression %u not supported!", ih->compression);
        return deallocate(bm, EFTYPE);
    }

    bm->opl = (((bm->width * bm->bpp) + 31) & ~31) >> 3;
    if (MAX_CHUNK < bm->opl)
    {
        LOG("exit, %u octets per line not supported!", bm->opl);
        return deallocate(bm, EFTYPE);
    }

    bm->offset = fh->off_bits;
    bm->chunk_top = 0;
    bm->chunk_height = MAX_CHUNK / bm->opl;
    LOG("exit, %d lines per chunk", bm->chunk_height);
    return true;
}

bool
bmp_load_bitmap(gfx_bitmap *bm, hasset asset)
{
    LOG("entry, bm: %p, asset: %p", (void *)bm, (void *)asset);

    if (NULL != bm->bits)
    {
        bm->offset += bm->opl * bm->chunk_height;
        bm->chunk_top += bm->chunk_height;
    }
    else if (!prepare(bm, asset))
    {
        return false;
    }

    if (abs(bm->height) < (bm->chunk_top + bm->chunk_height))
    {
        bm->chunk_height = abs(bm->height) - bm->chunk_top;
    }

    pal_read_asset(asset, bm->bits, bm->offset, bm->opl * bm->chunk_height);
    return true;
}

void
bmp_dispose_bitmap(gfx_bitmap *bm)
{
    LOG("entry, bm: %p", (void *)bm);

    free(bm->bits);
    bm->bits = NULL;
}
