#include <fmt/utf8.h>

#include "sld_impl.h"

#ifndef UTF8_NATIVE
static int
_convert_text(const char *str, sld_entry *inout)
{
    inout->length = utf8_encode(inout->content, inout->content, gfx_wctob);
    if (0 > inout->length)
    {
        __sld_errmsgcpy(inout, IDS_BADENCODING);
        return SLD_ARGERR;
    }

    return 0;
}
#endif

int
__sld_execute_text(sld_entry *sld)
{
    uint16_t x, y = sld->posy;

    switch (sld->posx)
    {
    case SLD_ALIGN_CENTER:
        x = (LINE_WIDTH - sld->length) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = LINE_WIDTH - sld->length;
        break;
    default:
        x = sld->posx;
    }

    if (!gfx_draw_text(sld->content, x, y))
    {
        __sld_errmsgcpy(sld, IDS_UNSUPPORTED);
        return SLD_SYSERR;
    }

    return 0;
}

int
__sld_load_text(const char *str, sld_entry *out)
{
    const char *cur = str;

    __sld_try_load(__sld_load_position, cur, out);
    __sld_try_load(__sld_load_content, cur, out);
#ifdef UTF8_NATIVE
    out->length = utf8_strlen(out->content);
#else
    __sld_try_load(_convert_text, cur, out);
#endif

    return cur - str;
}
