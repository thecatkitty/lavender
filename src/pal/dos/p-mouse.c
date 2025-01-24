#include <arch/dos/msmouse.h>
#include <gfx.h>
#include <pal.h>

#include "dos.h"

gfx_dimensions dos_cell;
bool           dos_mouse = false;

void
pal_enable_mouse(void)
{
    if (dos_mouse)
    {
        msmouse_show();
    }
}

void
pal_disable_mouse(void)
{
    if (dos_mouse)
    {
        msmouse_hide();
    }
}

uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y)
{
    if (!dos_mouse)
    {
        return 0;
    }

    uint16_t lowx, lowy, status;

    status = msmouse_get_status(&lowx, &lowy);
    *x = lowx / dos_cell.width;
    *y = lowy / dos_cell.height;

    return status;
}
