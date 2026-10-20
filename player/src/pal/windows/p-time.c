#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

bool  windows_no_stall = false;
DWORD windows_start_time = 0;

uint32_t
pal_get_counter(void)
{
    return timeGetTime() - windows_start_time;
}

uint32_t
pal_get_ticks(unsigned ms)
{
    return ms;
}

void
pal_sleep(unsigned ms)
{
    LOG("entry, ms: %u", ms);

    Sleep(ms);
}

void
pal_stall(int ms)
{
    if (0 < ms)
    {
        windows_no_stall = false;
        SetTimer(windows_wnd, 0, ms, NULL);
        return;
    }

    windows_no_stall = (0 == ms);
    SetTimer(windows_wnd, 0, 20, NULL);
}
