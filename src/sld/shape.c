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
