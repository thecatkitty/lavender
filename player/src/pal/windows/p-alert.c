#include <stdio.h>

#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

void
pal_alert(const char *text, int error)
{
    WCHAR msg[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, text, -1, msg, MAX_PATH);

    if (error)
    {
        WCHAR error_msg[32];
        swprintf(error_msg,
#if !defined(_MSC_VER) || (_MSC_VER > 1400)
                 lengthof(error_msg),
#endif
                 L"\nerror %d", error);
        wcsncat(msg, error_msg, MAX_PATH - wcslen(msg));
    }

    MessageBoxW(windows_wnd, msg, L"Lavender",
                error ? MB_ICONERROR : MB_ICONEXCLAMATION);
}
