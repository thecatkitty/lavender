#include <fmt/bmp.h>
#include <fmt/pbm.h>
#include <pal.h>
#include <sld.h>

#include "sld_impl.h"

static hasset
_find_best_bitmap(char *pattern)
{
    int par, offset;

    const char *hex = "0123456789ABCDEF";
    char       *placeholder = strstr(pattern, "<>");
    if (NULL == placeholder)
    {
        return pal_open_asset(pattern, O_RDONLY);
    }

    par = (int)gfx_get_pixel_aspect();
    offset = 0;
    while ((0 <= (par + offset)) || (255 >= (par + offset)))
    {
        if ((0 <= (par + offset)) && (255 >= (par + offset)))
        {
            hasset asset;
            placeholder[0] = hex[(par + offset) / 16];
            placeholder[1] = hex[(par + offset) % 16];
            asset = pal_open_asset(pattern, O_RDONLY);
            if (NULL != asset)
            {
                return asset;
            }
        }

        if (0 <= offset)
        {
            offset++;
        }
        offset = -offset;
    }

    errno = ENOENT;
    return NULL;
}

int
__sld_execute_bitmap(sld_entry *sld)
{
    float          scale;
    gfx_bitmap     bm;
    gfx_dimensions screen;
    hasset         bitmap = _find_best_bitmap(sld->content);
    int            x, y;

    if (NULL == bitmap)
    {
        return SLD_SYSERR;
    }

    if (bmp_is_format(bitmap))
    {
        if (!bmp_load_bitmap(&bm, bitmap))
        {
            return SLD_SYSERR;
        }
    }
    else if (!pbm_load_bitmap(&bm, bitmap))
    {
        return SLD_SYSERR;
    }

    gfx_get_screen_dimensions(&screen);
    scale = gfx_get_scale();

    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = ((float)screen.width / scale - bm.width) / 2 * scale;
        break;
    case SLD_ALIGN_RIGHT:
        x = ((float)screen.width / scale - bm.width) * scale;
        break;
    default:
        x = (int)((int32_t)sld->posx * screen.width / SLD_VIEWBOX_WIDTH);
    }

    y = (int)((int32_t)sld->posy * screen.height / SLD_VIEWBOX_HEIGHT);

    if (!gfx_draw_bitmap(&bm, x, y))
    {
        return SLD_SYSERR;
    }

    pal_close_asset(bitmap);
    return 0;
}

int
__sld_load_bitmap(const char *str, sld_entry *out)
{
    const char *cur = str;

    __sld_try_load(__sld_load_position, cur, out);
    __sld_try_load(__sld_load_content, cur, out);

    return cur - str;
}
