#include <ctype.h>
#include <stdlib.h>

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
pbm_load_bitmap(gfx_bitmap *bm, hasset asset)
{
    int count;
    uint16_t width, height;

    // Skip magic
    char *data = pal_get_asset_data(asset);
    data += sizeof(uint16_t);

    count = _load_number(data, &width);
    if (0 > count)
        return false;

    bm->opl = width / 8 + ((width % 8) ? 1 : 0);
    data += count;

    count = _load_number(data, &height);
    if (0 > count)
        return false;
    data += count;

    bm->width = width;
    bm->height = height;
    bm->bits = (char *)data + 1;
    bm->planes = 1;
    bm->bpp = 1;

    return true;
}
