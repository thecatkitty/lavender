#include "sld_impl.h"

int
__sld_execute_rectangle(sld_entry *sld)
{
    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (__sld_screen.width - sld->shape.dimensions.width) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = __sld_screen.height - sld->shape.dimensions.height;
        break;
    default:
        x = sld->posx;
    }

    bool (*draw)(gfx_dimensions *, uint16_t, uint16_t, gfx_color);
    draw =
        (SLD_TYPE_RECT == sld->type) ? gfx_draw_rectangle : gfx_fill_rectangle;
    return draw(&sld->shape.dimensions, x, y, sld->shape.color)
               ? 0
               : ERR_KER_UNSUPPORTED;
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
        out->shape.color = GFX_COLOR_BLACK;
        break;
    case 'G':
        out->shape.color = GFX_COLOR_GRAY;
        break;
    default:
        out->shape.color = GFX_COLOR_WHITE;
    }
    out->shape.dimensions.width = width;
    out->shape.dimensions.height = height;
    cur++;

    return cur - str;
}
