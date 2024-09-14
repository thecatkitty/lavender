#if defined(_WIN32)
#include <platform/windows.h>
#endif

#include <string.h>

#include "sld_impl.h"

#if defined(CONFIG_SDL2)
#include <platform/sdl2arch.h>
#endif

static uint16_t
_get(const char *name)
{
    if (0 == strcmp("gfx.colorful", name))
    {
        return gfx_get_color_depth() > 2;
    }

    return UINT16_MAX;
}

static uint16_t
_set(const char *name, const char *value)
{
    if (0 == strcmp("gfx.title", name))
    {
#if defined(CONFIG_SDL2)
        sdl2arch_set_window_title(value);
        return 1;
#elif defined(_WIN32)
        windows_set_window_title(value);
        return 1;
#else
        return 0;
#endif
    }

    return UINT16_MAX;
}

int
__sld_execute_query(sld_entry *sld)
{
    char *value = strchr(sld->content, ':');
    if (value)
    {
        *value++ = 0;
        __sld_accumulator = _set(sld->content, value);
    }
    else
    {
        __sld_accumulator = _get(sld->content);
    }
    return 0;
}
