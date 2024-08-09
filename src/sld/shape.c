#include "sld_impl.h"

typedef struct
{
    gfx_dimensions dimensions;
    gfx_color      color;
} shape_content;
#define CONTENT(sld) ((shape_content *)(&sld->content))

static void
_translate(uint16_t *x, uint16_t *y, gfx_dimensions *dims)
{
    int32_t w = dims->width;
    int32_t h = dims->height;
    int32_t xend = ((int32_t)*x + w) * __sld_screen.width / SLD_VIEWBOX_WIDTH;
    int32_t yend = ((int32_t)*y + h) * __sld_screen.height / SLD_VIEWBOX_HEIGHT;
    *x = (uint16_t)((int32_t)*x * __sld_screen.width / SLD_VIEWBOX_WIDTH);
    *y = (uint16_t)((int32_t)*y * __sld_screen.height / SLD_VIEWBOX_HEIGHT);
    dims->width = xend - *x;
    dims->height = yend - *y;
}

int
__sld_execute_rectangle(sld_entry *sld)
{
    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (SLD_VIEWBOX_WIDTH - CONTENT(sld)->dimensions.width) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = SLD_VIEWBOX_WIDTH - CONTENT(sld)->dimensions.width;
        break;
    default:
        x = sld->posx;
    }

    _translate(&x, &y, &CONTENT(sld)->dimensions);

    bool (*draw)(gfx_rect *, gfx_color);
    gfx_rect rect = {x, y, CONTENT(sld)->dimensions.width,
                     CONTENT(sld)->dimensions.height};
    draw =
        (SLD_TYPE_RECT == sld->type) ? gfx_draw_rectangle : gfx_fill_rectangle;
    if (!draw(&rect, CONTENT(sld)->color))
    {
        __sld_errmsgcpy(sld, IDS_UNSUPPORTED);
        return SLD_SYSERR;
    }

    return 0;
}

int
__sld_load_shape(const char *str, sld_entry *out)
{
    const char *cur = str;
    uint16_t    width, height;

    __sld_try_load(__sld_load_position, cur, out);

    cur += __sld_loadu(cur, &width);
    cur += __sld_loadu(cur, &height);

    switch (*cur)
    {
    case 'B':
        CONTENT(out)->color = GFX_COLOR_BLACK;
        break;
    case 'W':
        CONTENT(out)->color = GFX_COLOR_WHITE;
        break;
    case 'G':
        CONTENT(out)->color = GFX_COLOR_GRAY;
        break;
    default:
        __sld_try_load(__sld_load_content, cur, out);
        CONTENT(out)->color = gfx_get_color(out->content);
        if (GFX_COLOR_UNKNOWN == CONTENT(out)->color)
        {
            CONTENT(out)->color = GFX_COLOR_GRAY;
        }
    }
    CONTENT(out)->dimensions.width = width;
    CONTENT(out)->dimensions.height = height;
    cur++;

    return cur - str;
}
