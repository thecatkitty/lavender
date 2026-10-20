#include <string.h>

#include <gfx.h>
#include <pal.h>

typedef struct
{
    const char *name;
    gfx_color   color;
} color_name;

static const color_name NAMED_COLORS[] = {
    {"aqua", GFX_COLOR_CYAN},       {"black", GFX_COLOR_BLACK},
    {"blue", GFX_COLOR_BLUE},       {"cyan", GFX_COLOR_CYAN},
    {"fuchsia", GFX_COLOR_FUCHSIA}, {"gray", GFX_COLOR_GRAY},
    {"green", GFX_COLOR_GREEN},     {"lime", GFX_COLOR_LIME},
    {"maroon", GFX_COLOR_MAROON},   {"navy", GFX_COLOR_NAVY},
    {"olive", GFX_COLOR_OLIVE},     {"purple", GFX_COLOR_PURPLE},
    {"red", GFX_COLOR_RED},         {"silver", GFX_COLOR_SILVER},
    {"teal", GFX_COLOR_TEAL},       {"white", GFX_COLOR_WHITE},
    {"yellow", GFX_COLOR_YELLOW},
};

gfx_color
gfx_get_color(const char *str)
{
    size_t begin = 0;
    size_t end = lengthof(NAMED_COLORS) - 1;
    size_t i;

    int result = -1;
    while (begin <= end)
    {
        i = (begin + end) / 2;

        result = strcmp(NAMED_COLORS[i].name, str);
        if (0 < result)
        {
            end = i - 1;
        }
        else if (0 > result)
        {
            begin = i + 1;
        }
        else
        {
            return NAMED_COLORS[i].color;
        }
    }

    LOG("exit, unknown!");
    return GFX_COLOR_UNKNOWN;
}
