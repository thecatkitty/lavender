#include <cstring>

#include "widgets.hpp"

using namespace ui;

checkbox::checkbox(const encui_page &page, encui_field &field)
    : widget{page, field}, box_{}
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    rect_.width = TEXT_WIDTH;
    rect_.height = 2;

    box_.left = glyph.width * 2 / 3 + 1;
    box_.top = (glyph.height - glyph.width) / 2;
    box_.width = box_.height = glyph.width + 4;
}

void
checkbox::draw()
{
    char buffer[GFX_COLUMNS * 4] = "   ";
    encui_direct_load_string(&field_, buffer + 3, sizeof(buffer) - 8);
    std::strcat(buffer, " [F8]");
    encui_direct_print(rect_.top, buffer);

    mark(ENCUIFF_CHECKED & field_.flags);
}

int
checkbox::click(int x, int y)
{
    field_.flags ^= ENCUIFF_CHECKED;

#ifdef __ia16__
    mark(true);
#else
    mark(ENCUIFF_CHECKED & field_.flags);
#endif

    return 0;
}

void
checkbox::mark(bool checked)
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    gfx_rect box = box_;
    gfx_rect pos = get_position();
    box.left += pos.left * glyph.width;
    box.top += pos.top * glyph.height;

    gfx_draw_rectangle(&box, GFX_COLOR_BLACK);
#ifndef __ia16__
    gfx_fill_rectangle(&box, GFX_COLOR_WHITE);
#endif

    if (checked)
    {
        gfx_draw_text("x", pos.left + 1, pos.top);
    }
}
