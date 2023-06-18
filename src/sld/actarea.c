#include "sld_impl.h"

typedef struct
{
    gfx_dimensions dimensions;
    uint16_t       tag;
} active_area_content;
#define CONTENT(sld) ((active_area_content *)(&sld->content))

#define MAX_AREAS 16

static struct
{
    uint16_t left;
    uint16_t top;
    uint16_t width;
    uint16_t height;
    uint16_t tag;
} _areas[MAX_AREAS];

static bool
_add_area(
    uint16_t left, uint16_t top, uint16_t width, uint16_t height, uint16_t tag)
{
    for (int i = 0; i < MAX_AREAS; i++)
    {
        if ((_areas[i].left == left) && (_areas[i].top == top) &&
            (_areas[i].width == width) && (_areas[i].height == height))
        {
            _areas[i].tag = tag;
            return true;
        }

        if (0 == _areas[i].tag)
        {
            _areas[i].left = left;
            _areas[i].top = top;
            _areas[i].width = width;
            _areas[i].height = height;
            _areas[i].tag = tag;
            return true;
        }
    }

    return false;
}

int
__sld_execute_active_area(sld_entry *sld)
{
    if ((0 == sld->posx) && (0 == sld->posy) &&
        (0 == CONTENT(sld)->dimensions.width) && (0 == CONTENT(sld)->tag))
    {
        for (int i = 0; i < MAX_AREAS; i++)
        {
            _areas[i].tag = 0;
        }

        return 0;
    }

    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (LINE_WIDTH - CONTENT(sld)->dimensions.width) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = LINE_WIDTH - CONTENT(sld)->dimensions.width;
        break;
    default:
        x = sld->posx;
    }

    if (!_add_area(x, y, CONTENT(sld)->dimensions.width,
                   CONTENT(sld)->dimensions.height, CONTENT(sld)->tag))
    {
        __sld_errmsgcpy(sld, IDS_TOOMANYAREAS);
        return SLD_ARGERR;
    }

    return 0;
}

int
__sld_load_active_area(const char *str, sld_entry *out)
{
    const char *cur = str;
    uint16_t    width, height, tag;

    __sld_try_load(__sld_load_position, cur, out);

    cur += __sld_loadu(cur, &width);
    cur += __sld_loadu(cur, &height);
    cur += __sld_loadu(cur, &tag);

    CONTENT(out)->dimensions.width = width;
    CONTENT(out)->dimensions.height = height;
    CONTENT(out)->tag = tag;
    cur++;

    return cur - str;
}

int
__sld_retrieve_active_area_tag(uint16_t x, uint16_t y)
{
    for (int i = 0; i < MAX_AREAS; i++)
    {
        if (_areas[i].left > x)
        {
            continue;
        }

        if (_areas[i].top > y)
        {
            continue;
        }

        if (x - _areas[i].left > _areas[i].width)
        {
            continue;
        }

        if (y - _areas[i].top > _areas[i].height)
        {
            continue;
        }

        return _areas[i].tag;
    }

    return 0;
}
