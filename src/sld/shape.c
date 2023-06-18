#include "sld_impl.h"

typedef struct
{
    gfx_dimensions dimensions;
    gfx_color      color;
} shape_content;
#define CONTENT(sld) ((shape_content *)(&sld->content))

int
__sld_execute_rectangle(sld_entry *sld)
{
    uint16_t x, y = sld->posy;
    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (__sld_screen.width - CONTENT(sld)->dimensions.width) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = __sld_screen.width - CONTENT(sld)->dimensions.width;
        break;
    default:
        x = sld->posx;
    }

    bool (*draw)(gfx_dimensions *, uint16_t, uint16_t, gfx_color);
    draw =
        (SLD_TYPE_RECT == sld->type) ? gfx_draw_rectangle : gfx_fill_rectangle;
    if (!draw(&CONTENT(sld)->dimensions, x, y, CONTENT(sld)->color))
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
    case 'G':
        CONTENT(out)->color = GFX_COLOR_GRAY;
        break;
    default:
        CONTENT(out)->color = GFX_COLOR_WHITE;
    }
    CONTENT(out)->dimensions.width = width;
    CONTENT(out)->dimensions.height = height;
    cur++;

    return cur - str;
}
