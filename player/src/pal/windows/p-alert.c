#include <stdio.h>

#include <arch/windows.h>
#include <compat/wchar.h>
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
        swprintf_c(error_msg, lengthof(error_msg), L"\nerror %d", error);
        wcsncat(msg, error_msg, MAX_PATH - wcslen(msg) - 1);
    }

    MessageBoxW(windows_wnd, msg, L"Lavender",
                error ? MB_ICONERROR : MB_ICONEXCLAMATION);
}
