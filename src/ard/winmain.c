#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>
#include <ard/ui.h>
#include <ard/version.h>

#include "resource.h"

static int
die_with_rundos(_Inout_z_ char *message, _In_ const ardc_config *cfg)
{
    bool has_rundos = arda_rundos_available(cfg);
    if (has_rundos)
    {
        size_t length = strlen(message);
        message[length++] = ' ';
        LoadString(NULL, IDS_DIERUNDOS, message + length,
                   ARDC_LENGTH_LONG - length - 1);
    }

    if (IDYES == ardui_msgbox(message, has_rundos ? (MB_ICONWARNING | MB_YESNO)
                                                  : (MB_ICONERROR | MB_OK)))
    {
        return arda_rundos(cfg);
    }

    return 0;
}

int WINAPI
WinMain(_In_ HINSTANCE     instance,
        _In_opt_ HINSTANCE prev_instance,
        _In_ PSTR          cmd_line,
        _In_ int           cmd_show)
{
    ardc_config *cfg = NULL;
    char         message[ARDC_LENGTH_LONG] = "";

    MSG msg;

    LoadString(NULL, IDS_DEFNAME, message, ARRAYSIZE(message));
    ardui_set_title(message);

    if (NULL == (cfg = ardc_load()))
    {
        // cannot load configuration
        LoadString(NULL, IDS_ECONFIG, message, ARRAYSIZE(message));
        ardui_msgbox(message, MB_ICONERROR | MB_OK);
        return 0;
    }

    ardui_set_title(cfg->name);

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

        return die_with_rundos(message, cfg);
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

        return die_with_rundos(message, cfg);
    }

    // check the service pack version
    if (cfg->ossp > ardv_windows_get_servicepack())
    {
        char     format[ARDC_LENGTH_MID] = "";
        bool     is_nt = ardv_windows_get_version();
        uint16_t winver = ardv_windows_get_version();

        LoadString(NULL, is_nt ? IDS_OLDSP : IDS_OLDOSR, format,
                   ARRAYSIZE(format));
        sprintf(message, format, cfg->name,
                ardv_windows_get_spname(winver, cfg->ossp, is_nt),
                ardv_windows_get_name(winver, is_nt));

        return die_with_rundos(message, cfg);
    }

    arda_run(cfg);

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}
