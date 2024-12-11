#include <cstring>

#include "widgets.hpp"

using namespace ui;

void
label::draw()
{
    char buffer[GFX_COLUMNS * 4];
    if (ENCUIFF_DYNAMIC & field_.flags)
    {
        std::strncpy(buffer, reinterpret_cast<const char *>(field_.data),
                     sizeof(buffer));
    }
    else
    {
        pal_load_string(field_.data, buffer, sizeof(buffer));
    }

    rect_.width = GFX_COLUMNS - 2;
    rect_.height = encui_direct_print(rect_.top, buffer) + 1;
}
