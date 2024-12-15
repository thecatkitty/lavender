#include <cstring>

#include "widgets.hpp"

using namespace ui;

button::button(const encui_page &page, encui_field &field) : widget{page, field}
{
    rect_.width = 11;
    rect_.height = 3;
}

void
button::draw()
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    auto field =
        gfx_rect{rect_.left * glyph.width + (glyph.width / 2),
                 rect_.top * glyph.height + (glyph.height * 3 / 4),
                 (rect_.width - 2) * glyph.width, glyph.height * 3 / 2};

    char buff[GFX_COLUMNS / 2];
    if (ENCUIFF_DYNAMIC & field_.flags)
    {
        std::strncpy(buff, reinterpret_cast<const char *>(field_.data),
                     sizeof(buff));
    }
    else
    {
        pal_load_string(field_.data, buff, sizeof(buff));
    }

#ifndef UTF8_NATIVE
    utf8_encode(buff, buff, pal_wctob);
#endif

    gfx_rect inner = {field.left - 1, field.top, field.width + 2, field.height};
    gfx_fill_rectangle(&field, GFX_COLOR_WHITE);
    gfx_draw_rectangle(&field, GFX_COLOR_BLACK);
    gfx_draw_rectangle(&inner, GFX_COLOR_BLACK);

    auto pos = get_position();
    gfx_draw_text(buff, pos.left + (8 - strlen(buff)) / 2 + 1, pos.top + 1);
}

int
button::click(int x, int y)
{
    return field_.data;
}
