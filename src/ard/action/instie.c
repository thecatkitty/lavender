#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>
#include <ard/ui.h>

#include "../resource.h"

int
arda_instie(_In_ const ardc_config *cfg)
{
    char format[ARDC_LENGTH_LONG] = "";
    char message[ARDC_LENGTH_LONG] = "";

    if (32 <= WinExec(cfg->ie_install, SW_SHOWNORMAL))
    {
        PostQuitMessage(0);
        return 0;
    }

    LoadString(NULL, IDS_EINSTNOF, format, ARRAYSIZE(format));
    sprintf(message, format, "Internet Explorer");
    ardui_msgbox(message, MB_ICONERROR | MB_OK);
    PostQuitMessage(0);
    return 1;
}
