#include "widgets.hpp"

using namespace ui;

bitmap::bitmap(encui_field &field) : widget{field}
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    auto &bm = *reinterpret_cast<gfx_bitmap *>(field_.data);
    rect_.width = (bm.width + glyph.width - 1) / glyph.width;
    rect_.height = (bm.height + glyph.height - 1) / glyph.height + 1;
}

void
bitmap::draw()
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    auto &bm = *reinterpret_cast<gfx_bitmap *>(field_.data);
    auto  pos = get_position();

    auto x = pos.left;
    if (ENCUIFF_CENTER == (ENCUIFF_ALIGN & field_.flags))
    {
        x += (TEXT_WIDTH - rect_.width) / 2;
    }
    else if (ENCUIFF_RIGHT == (ENCUIFF_ALIGN & field_.flags))
    {
        x += TEXT_WIDTH - rect_.width;
    }
    x *= glyph.width;

    auto y = pos.top * glyph.height;

    gfx_draw_bitmap(&bm, x, y);
}
