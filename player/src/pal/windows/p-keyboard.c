#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

WPARAM windows_keycode;

uint16_t
pal_get_keystroke(void)
{
    uint16_t keycode = windows_keycode;
    windows_keycode = 0;
    return keycode;
}
