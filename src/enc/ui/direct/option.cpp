#include <algorithm>
#include <cstdio>
#include <cstring>

#include "widgets.hpp"

using namespace ui;

option::option(encui_field &field) : widget{field}
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    rect_.width = TEXT_WIDTH;
    rect_.height = 2;
}

void
option::draw()
{
    char buffer[GFX_COLUMNS * 4] = "   ";
    encui_direct_load_string(&field_, buffer + 3, sizeof(buffer) - 8);

    auto page = get_page();
    if (nullptr != page)
    {
        auto pos =
            std::count_if(page->fields, &field_, [](const encui_field &field) {
                return ENCUIFT_OPTION == field.type;
            });

        if (7 > pos)
        {
            std::sprintf(buffer + strlen(buffer), " [F%d]", int(pos + 1));
        }
    }

    encui_direct_print(rect_.top, buffer);

    mark(ENCUIFF_CHECKED & field_.flags);
}

int
option::click(int x, int y)
{
    if (nullptr == parent_)
    {
        return 0;
    }

    for (auto &widget : *reinterpret_cast<panel *>(parent_))
    {
        if (ENCUIFT_OPTION != widget->get_model().type)
        {
            continue;
        }

        auto &that = *reinterpret_cast<option *>(widget.get());
        that.set(false);
    }

    set(true);
    return 0;
}

void
option::mark(bool checked)
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    gfx_rect pos = get_position();
    gfx_rect box = gfx_rect{(pos.left + 1) * glyph.width,
                            pos.top * glyph.height, glyph.width, glyph.height};
    gfx_fill_rectangle(&box, GFX_COLOR_WHITE);

    char buff[4];
    strcpy(buff, "○");
#ifndef UTF8_NATIVE
    utf8_encode(buff, buff, pal_wctob);
#endif
    gfx_draw_text(buff, pos.left + 1, pos.top);
    if (checked)
    {
#ifdef __ia16__
        strcpy(buff, "•");
#else
        strcpy(buff, "x");
#endif
#ifndef UTF8_NATIVE
        utf8_encode(buff, buff, pal_wctob);
#endif
        gfx_draw_text(buff, pos.left + 1, pos.top);
    }
}

void
option::set(bool checked)
{
    mark(checked);
    if (checked)
    {
        field_.flags |= ENCUIFF_CHECKED;
    }
    else
    {
        field_.flags &= ~ENCUIFF_CHECKED;
    }
}
