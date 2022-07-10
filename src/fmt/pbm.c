#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

#include <api/dos.h>
#include <fmt/pbm.h>

static int
_load_number(const char *str, uint16_t *out)
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
            errno = EFTYPE;
            return -1;
        }

        *out = *out * 10 + (*str - '0');
        str++;
    }

    return str - start;
}

bool
pbm_is_format(hasset asset)
{
    return PBM_RAW_MAGIC == *(uint16_t *)pal_get_asset_data(asset);
}

bool
pbm_load_bitmap(GFX_BITMAP *bm, hasset asset)
{
    int count;

    // Skip magic
    char *data = pal_get_asset_data(asset);
    data += sizeof(uint16_t);

    count = _load_number(data, &bm->Width);
    if (0 > count)
        return false;

    bm->WidthBytes = bm->Width / 8 + ((bm->Width % 8) ? 1 : 0);
    data += count;

    count = _load_number(data, &bm->Height);
    if (0 > count)
        return false;
    data += count;

    bm->Bits = (char *)data + 1;
    bm->Planes = 1;
    bm->BitsPerPixel = 1;

    return true;
}
