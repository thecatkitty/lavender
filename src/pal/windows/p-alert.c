#include <stdio.h>

#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

void
pal_alert(const char *text, int error)
{
    WCHAR msg[MAX_PATH];
    if (error)
    {
        swprintf(msg,
#if !defined(_MSC_VER) || (_MSC_VER > 1400)
                 MAX_PATH,
#endif
                 L"" FMT_AS L"\nerror %d", text, error);
    }
    else
    {
        MultiByteToWideChar(CP_UTF8, 0, text, -1, msg, MAX_PATH);
    }

    MessageBoxW(windows_wnd, msg, L"Lavender",
                error ? MB_ICONERROR : MB_ICONEXCLAMATION);
}
