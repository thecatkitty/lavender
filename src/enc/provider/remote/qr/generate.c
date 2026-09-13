#include <gfx.h>

#include "encqr.h"

#include <QR-Code-generator/c/qrcodegen.h>

static void
_set_pixel(shiz_bitmap *bm, int x, int y, int xscale, int yscale, bool value)
{
    uint8_t *line = (uint8_t *)bm->pixels + y * yscale * bm->stride;
    int      sx, sy;

    for (sy = 0; sy < yscale; sy++)
    {
        for (sx = 0; sx < xscale; sx++)
        {
            uint8_t *cell = line + (x * xscale + sx) / 8;
            if (value)
            {
                *cell &= ~(0x80 >> ((x * xscale + sx) % 8));
            }
        }

        line += bm->stride;
    }
}

bool
encqr_generate(const char *str, shiz_bitmap *bm)
{
    uint8_t buffer[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];
    uint8_t qr[qrcodegen_BUFFER_LEN_FOR_VERSION(10)];
    int     size, xscale, yscale, x, y;

    if (!qrcodegen_encodeText(str, buffer, qr, qrcodegen_Ecc_MEDIUM, 1, 10,
                              qrcodegen_Mask_AUTO, true))
    {
        return false;
    }

    if (NULL == (bm->pixels = (uint8_t *)malloc(bm->size.y * bm->stride)))
    {
        return false;
    }

    size = qrcodegen_getSize(qr);
    xscale = bm->size.x / size;
    yscale = xscale * 64 / gfx_get_pixel_aspect();

    memset((void *)bm->pixels, 0xFF, bm->stride * bm->size.y);
    for (y = 0; y < size; y++)
    {
        for (x = 0; x < size; x++)
        {
            _set_pixel(bm, x, y, xscale, yscale, qrcodegen_getModule(qr, x, y));
        }
    }

    bm->size.x = size * xscale;
    bm->size.y = size * yscale;

    return true;
}
