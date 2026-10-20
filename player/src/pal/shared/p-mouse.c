#include <pal.h>

#include <evtmouse.h>

static bool     enabled_ = false;
static uint16_t x_, y_, buttons_;

void
pal_enable_mouse(void)
{
    if (enabled_)
    {
        // Prevent log flood on input loops
        return;
    }

    LOG("entry");

    enabled_ = true;
    return;
}

void
pal_disable_mouse(void)
{
    if (!enabled_)
    {
        // Prevent log flood on input loops
        return;
    }

    LOG("entry");

    enabled_ = false;
    buttons_ = 0;
    return;
}

uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y)
{
    if (!enabled_)
    {
        return 0;
    }

    *x = x_;
    *y = y_;
    if (buttons_)
    {
        LOG("x: %u, y: %u, buttons: %#x", *x, *y, buttons_);
    }
    return buttons_;
}

void
evtmouse_press(uint16_t buttons)
{
    if (enabled_)
    {
        buttons_ |= buttons;
    }
}

void
evtmouse_release(uint16_t buttons)
{
    buttons_ &= ~buttons;
}

void
evtmouse_set_position(uint16_t x, uint16_t y)
{
    x_ = x;
    y_ = y;
}
