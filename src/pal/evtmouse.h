#ifndef _EVTMOUSE_H_
#define _EVTMOUSE_H_

#include <gfx.h>
#include <pal.h>

static bool           _mouse_enabled = false;
static uint16_t       _mouse_x, _mouse_y, _mouse_buttons;

void
pal_enable_mouse(void)
{
    LOG("entry");

    _mouse_enabled = true;
    return;
}

void
pal_disable_mouse(void)
{
    LOG("entry");

    _mouse_enabled = false;
    _mouse_buttons = 0;
    return;
}

uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y)
{
    if (!_mouse_enabled)
    {
        return 0;
    }

    *x = _mouse_x;
    *y = _mouse_y;
    if (_mouse_buttons)
    {
        LOG("x: %u, y: %u, buttons: %#x", *x, *y, _mouse_buttons);
    }
    return _mouse_buttons;
}

#endif // _EVTMOUSE_H_
