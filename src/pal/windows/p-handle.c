#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

bool
pal_handle(void)
{
    MSG msg;
    do
    {
        if (windows_no_stall && !PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            return true;
        }

        if (!windows_no_stall && !GetMessageW(&msg, NULL, 0, 0))
        {
            return false;
        }

        if (WM_QUIT == msg.message)
        {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    } while (windows_no_stall);

    return true;
}
