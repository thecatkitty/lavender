#include <string.h>

#include "sld_impl.h"

int
__sld_execute_query(sld_entry *sld)
{
    if (0 == strcmp("gfx.colorful", sld->content))
    {
#ifdef GFX_COLORFUL
        __sld_accumulator = 1;
#else
        __sld_accumulator = 0;
#endif
        return 0;
    }

    __sld_accumulator = UINT16_MAX;
    return 0;
}
