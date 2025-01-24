#include <arch/dos.h>

#include "impl.h"

char
pal_wctoa(uint16_t wc)
{
    if (0x80 > wc)
    {
        return wc;
    }

    gfx_glyph_data fdata = dos_font;
    while (wc > fdata->codepoint)
    {
        fdata++;
    }

    if (wc != fdata->codepoint)
    {
        return '?';
    }

    return fdata->base;
}

char
pal_wctob(uint16_t wc)
{
    if (0x80 > wc)
    {
        return wc;
    }

    uint8_t        local = 0x80;
    gfx_glyph_data fdata = dos_font;

    while (wc > fdata->codepoint)
    {
        if (0 != fdata->overlay)
        {
            local++;
        }
        fdata++;
    }

    if (wc != fdata->codepoint)
    {
        return '?';
    }

    if (0 == fdata->overlay)
    {
        return fdata->base;
    }

    return local;
}
