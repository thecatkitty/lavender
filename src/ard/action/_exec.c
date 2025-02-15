#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>
#include <ard/ui.h>

#include "../resource.h"

int
arda_exec(_In_ const ardc_config *cfg, _In_z_ const char *cmd)
{
    char  message[ARDC_LENGTH_LONG];
    UINT  status;
    char *argv0_end = strchr(cmd, ' ');

    if (32 > (status = WinExec(cmd, SW_SHOWNORMAL)))
    {
        char format[ARDC_LENGTH_LONG] = "";
        int  id = IDS_EEXECNORES;

        switch (status)
        {
        case ERROR_BAD_FORMAT:
            id = IDS_EEXECBADF;
            break;
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
            id = IDS_EEXECNOF;
            break;
        }

        if (argv0_end)
        {
            *argv0_end = 0;
        }

        LoadString(NULL, id, format, ARRAYSIZE(format));
        sprintf(message, format, cfg->name, cmd);
        ardui_msgbox(message, MB_ICONERROR | MB_OK);

        if (argv0_end)
        {
            *argv0_end = ' ';
        }

        return 1;
    }

    return 0;
}
