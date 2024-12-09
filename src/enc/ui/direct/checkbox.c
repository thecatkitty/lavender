#include "direct.h"

static gfx_rect _box = {0};
static gfx_rect _area;

static void
_draw_checkbox(encui_field *field)
{
    if (NULL == field)
    {
        return;
    }

#ifndef __ia16__
    gfx_fill_rectangle(&_box, GFX_COLOR_WHITE);

    if (ENCUIFF_CHECKED & field->flags)
#endif
    {
        gfx_draw_text("x", 2, GFX_LINES - 5);
    }
}

void
encui_direct_create_checkbox(encui_field *field, char *buffer, size_t size)
{
    buffer[0] = buffer[1] = buffer[2] = ' ';
    if (ENCUIFF_DYNAMIC & field->flags)
    {
        strncpy(buffer + 3, (const char *)field->data, size - 8);
    }
    else
    {
        pal_load_string(field->data, buffer + 3, size - 8);
    }
    strcat(buffer, " [F8]");
    encui_direct_print(GFX_LINES - 5, buffer);

    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    _box.width = _box.height = glyph.width + 4;
    _box.left = glyph.width * 5 / 3 + 1;
    _box.top =
        (GFX_LINES - 5) * glyph.height + (glyph.height - glyph.width) / 2;
    gfx_draw_rectangle(&_box, GFX_COLOR_BLACK);
    if (ENCUIFF_CHECKED & field->flags)
    {
        _draw_checkbox(field);
    }

    _area.left = glyph.width;
    _area.top = _box.top - (glyph.height / 2);
    _area.width = TEXT_WIDTH * glyph.width;
    _area.height = glyph.height * 2;
}

const gfx_rect *
encui_direct_get_checkbox_area(void)
{
    return &_area;
}

void
encui_direct_click_checkbox(encui_field *field)
{
    if (NULL == field)
    {
        return;
    }

    field->flags ^= ENCUIFF_CHECKED;
    _draw_checkbox(field);
}
