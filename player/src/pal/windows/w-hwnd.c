#include <arch/windows.h>

#include "impl.h"

HWND windows_wnd = NULL;

HWND
windows_get_hwnd(void)
{
    return windows_wnd;
}
