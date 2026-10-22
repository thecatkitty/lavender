#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>
#include <ard/ui.h>

#include "../resource.h"

#if defined(_MSC_VER)
#pragma warning(disable : 28159)
#endif

int
arda_exec(_In_ const ardc_config *cfg, _In_z_ const char *cmd)
{
    UINT status;

    if (32 > (status = WinExec(cmd, SW_SHOWNORMAL)))
    {
        char format[ARDC_LENGTH_LONG] = "";
        char message[ARDC_LENGTH_LONG];
        int  id = IDS_EEXECNORES;

        const char *argv0 = cmd;
        const char *argv0_end;

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

        if (*argv0 == '"')
        {
            ++argv0;
            argv0_end = strchr(argv0, '"');
        }
        else
        {
            argv0_end = strchr(argv0, ' ');
        }

        if (!argv0_end)
        {
            argv0_end = argv0 + strlen(argv0);
        }

        LoadString(NULL, id, format, ARRAYSIZE(format));
        snprintf(message, ARRAYSIZE(message), format, cfg->name,
                 (int)(argv0_end - argv0), argv0);
        ardui_msgbox(message, MB_ICONERROR | MB_OK);

        return 1;
    }

    return 0;
}
