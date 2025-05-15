#include <stdlib.h>
#include <string.h>

#include <gfx.h>

#include "sld_impl.h"

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
#if !defined(_WIN32)
    if (0 == strcmp("env.lang", name))
    {
        return (0 == setenv("LANG", value, 0)) ? 1 : 0;
    }
#endif

#if defined(__ia16__)
    if (0 == strcmp("env.tz", name))
    {
        return (0 == setenv("TZ", value, 0)) ? 1 : 0;
    }
#endif

    if (0 == strcmp("gfx.title", name))
    {
        return gfx_set_title(value) ? 1 : 0;
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
