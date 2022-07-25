#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <crg.h>
#include <dlg.h>
#include <fmt/pbm.h>
#include <gfx.h>
#include <pal.h>
#include <sld.h>

#include "sld_impl.h"

gfx_dimensions __sld_screen;

uint16_t __sld_accumulator = 0;

int
sld_execute_entry(sld_entry *sld)
{
    if (0 == __sld_screen.width)
    {
        gfx_get_screen_dimensions(&__sld_screen);
    }

    pal_sleep(sld->delay);

    switch (sld->type)
    {
    case SLD_TYPE_BLANK:
        return 0;
    case SLD_TYPE_LABEL:
        return 0;
    case SLD_TYPE_TEXT:
        return __sld_execute_text(sld);
    case SLD_TYPE_BITMAP:
        return __sld_execute_bitmap(sld);
    case SLD_TYPE_RECT:
    case SLD_TYPE_RECTF:
        return __sld_execute_rectangle(sld);
    case SLD_TYPE_PLAY:
        return __sld_execute_play(sld);
    case SLD_TYPE_WAITKEY:
        __sld_accumulator = bios_get_keystroke() >> 8;
        return 0;
    case SLD_TYPE_JUMP:
        return INT_MAX;
    case SLD_TYPE_JUMPE:
        return (__sld_accumulator == sld->posy) ? INT_MAX : 0;
    case SLD_TYPE_CALL:
        return __sld_execute_script_call(sld);
    }

    ERR(SLD_UNKNOWN_TYPE);
}
