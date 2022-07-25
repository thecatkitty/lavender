#include <errno.h>
#include <string.h>

#include <fmt/pbm.h>
#include <pal.h>
#include <sld.h>

#include "sld_impl.h"

static hasset
_find_best_bitmap(char *pattern)
{
    const char *hex = "0123456789ABCDEF";
    char       *placeholder = strstr(pattern, "<>");
    if (NULL == placeholder)
    {
        return pal_open_asset(pattern, O_RDONLY);
    }

    int par = (int)gfx_get_pixel_aspect();
    int offset = 0;

    while ((0 <= (par + offset)) || (255 >= (par + offset)))
    {
        if ((0 <= (par + offset)) && (255 >= (par + offset)))
        {
            placeholder[0] = hex[(par + offset) / 16];
            placeholder[1] = hex[(par + offset) % 16];
            hasset asset = pal_open_asset(pattern, O_RDONLY);
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

    return NULL;
}

bool
__sld_execute_bitmap(sld_entry *sld)
{
    hasset bitmap = _find_best_bitmap(sld->content);
    if (NULL == bitmap)
    {
        return -1;
    }

    gfx_bitmap bm;
    if (!pbm_load_bitmap(&bm, bitmap))
    {
        return -1;
    }

    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (LINE_WIDTH - bm.opl) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = LINE_WIDTH - bm.opl;
        break;
    default:
        x = sld->posx;
    }

    bool status = gfx_draw_bitmap(&bm, x, y);

    pal_close_asset(bitmap);
    return status;
}

int
__sld_load_bitmap(const char *str, sld_entry *out)
{
    const char *cur = str;

    __sld_try_load(__sld_load_position, cur, out);
    __sld_try_load(__sld_load_content, cur, out);

    return cur - str;
}
