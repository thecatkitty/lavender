#include <ctype.h>
#include <stdlib.h>

#include <api/dos.h>
#include <gfx.h>

// Raw Portable Bitmap                  P4
#define PBM_RAW_MAGIC 0x3450

static int
PbmLoadBitmap(const char *data, GFX_BITMAP *bm);

static int
PbmLoadU(const char *data, uint16_t *out);

int
GfxLoadBitmap(const void *data, GFX_BITMAP *bm)
{
    if (PBM_RAW_MAGIC == *(uint16_t *)data)
    {
        return PbmLoadBitmap((const char *)data, bm);
    }

    ERR(GFX_FORMAT);
}

int
PbmLoadBitmap(const char *data, GFX_BITMAP *bm)
{
    int count;

    // Skip magic
    data += sizeof(uint16_t);

    count = PbmLoadU(data, &bm->Width);
    if (0 > count)
        return count;

    bm->WidthBytes = bm->Width / 8 + ((bm->Width % 8) ? 1 : 0);
    data += count;

    count = PbmLoadU(data, &bm->Height);
    if (0 > count)
        return count;
    data += count;

    bm->Bits = (char *)data + 1;
    bm->Planes = 1;
    bm->BitsPerPixel = 1;

    return 0;
}

int
PbmLoadU(const char *str, uint16_t *out)
{
    const char *start = str;
    *out = 0;

    // Skip leading whitespaces
    while (isspace(*str))
    {
        str++;
    }

    while (!isspace(*str))
    {
        if ('#' == *str)
        {
            str++;
            // Skip comments
            while ('\n' != *str)
            {
                str++;
            }
            str++;
        }

        if (!isdigit(*str))
        {
            ERR(GFX_MALFORMED_FILE);
        }

        *out = *out * 10 + (*str - '0');
        str++;
    }

    return str - start;
}
