#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <ard/config.h>
#include <ard/version.h>

#include "resource.h"

static char title_[ARDC_LENGTH_MID] = "";

static int
show_msgbox(const char *str, unsigned style)
{
    return MessageBox(NULL, str, title_, style);
}

int WINAPI
WinMain(_In_ HINSTANCE     instance,
        _In_opt_ HINSTANCE prev_instance,
        _In_ PSTR          cmd_line,
        _In_ int           cmd_show)
{
    ardc_config *cfg = NULL;
    char         cmd[MAX_PATH] = "";
    char         message[ARDC_LENGTH_LONG] = "";

    MSG  msg;
    UINT status;

    LoadString(NULL, IDS_DEFNAME, title_, ARRAYSIZE(title_));

    if (NULL == (cfg = ardc_load()))
    {
        // cannot load configuration
        LoadString(NULL, IDS_ECONFIG, message, ARRAYSIZE(message));
        show_msgbox(message, MB_ICONERROR | MB_OK);
        return 0;
    }

    strcpy(title_, cfg->name);

    // check the processor level
    if (ardv_cpu_from_string(cfg->cpu, ARDV_CPU_I486) > ardv_cpu_get_level())
    {
        char cpu_desc[ARDC_LENGTH_MID] = "";
        char format[ARDC_LENGTH_MID] = "";

        LoadString(NULL, IDS_OLDCPU, format, ARRAYSIZE(format));
        LoadString(NULL,
                   IDS_CPUBASE + ardv_cpu_from_string(cfg->cpu, ARDV_CPU_I486),
                   cpu_desc, ARRAYSIZE(cpu_desc));
        sprintf(message, format, cfg->name, cpu_desc);

        show_msgbox(message, MB_ICONERROR | MB_OK);
        return 0;
    }

    // check the operating system version
    if ((ardv_windows_is_nt() ? cfg->winnt : cfg->win) >
        ardv_windows_get_version())
    {
        char format[ARDC_LENGTH_MID] = "";

        if (cfg->win)
        {
            LoadString(NULL, IDS_OLDWIN, format, ARRAYSIZE(format));
            sprintf(message, format, cfg->name,
                    ardv_windows_get_name(cfg->win, false),
                    ardv_windows_get_name(cfg->winnt, true));
        }
        else
        {
            LoadString(NULL, IDS_OLDWINNT, format, ARRAYSIZE(format));
            sprintf(message, format, cfg->name,
                    ardv_windows_get_name(cfg->winnt, true));
        }

        show_msgbox(message, MB_ICONERROR | MB_OK);
        return 0;
    }

    sprintf(cmd, "\"%s\"", cfg->run);
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

        LoadString(NULL, id, format, ARRAYSIZE(format));
        sprintf(message, format, cfg->name, cfg->run);
        show_msgbox(message, MB_ICONERROR | MB_OK);
        return 0;
    }

    PostQuitMessage(0);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
