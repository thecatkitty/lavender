#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>
#include <ard/ui.h>
#include <ard/version.h>

#include "resource.h"

static HWND frame_;

extern int
ard_check_dependencies(_Inout_ ardc_config *cfg);

extern _Ret_maybenull_ ardc_dependency *
ard_check_sources(_Inout_ ardc_config *cfg);

extern _Ret_maybenull_ ardc_source **
ard_get_sources(_Inout_ ardc_config *cfg);

static int
die_with_rundos_buff(_Inout_ char           *message,
                     _In_ size_t             capacity,
                     _In_ const ardc_config *cfg)
{
    bool has_rundos = arda_rundos_available(cfg);
    if (has_rundos)
    {
        size_t length = strlen(message);
        if (!isspace(message[length - 1]))
        {
            message[length++] = ' ';
        }
        LoadString(NULL, IDS_DIERUNDOS, message + length,
                   capacity - length - 1);
    }

    ardc_cleanup();
    if (IDYES == ardui_msgbox(message, has_rundos ? (MB_ICONWARNING | MB_YESNO)
                                                  : (MB_ICONERROR | MB_OK)))
    {
        return arda_rundos(cfg);
    }

    return 0;
}

static int
die_with_rundos(_Inout_ char *message, _In_ const ardc_config *cfg)
{
    return die_with_rundos_buff(message, ARDC_LENGTH_LONG, cfg);
}

int WINAPI
WinMain(_In_ HINSTANCE     instance,
        _In_opt_ HINSTANCE prev_instance,
        _In_ PSTR          cmd_line,
        _In_ int           cmd_show)
{
    ardc_config *cfg = NULL;
    bool         complete = false;
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

    // check Internet Explorer version - suppresses further checks
    if (cfg->ie_complete && ardv_ie_available() &&
        (cfg->ie_complete <= ardv_ie_get_version()))
    {
        complete = true;
    }

    // check the operating system version
    if (!complete && ((ardv_windows_is_nt() ? cfg->winnt : cfg->win) >
                      ardv_windows_get_version()))
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
    if (!complete && (cfg->ossp > ardv_windows_get_servicepack()))
    {
        char     format[ARDC_LENGTH_MID] = "";
        bool     is_nt = ardv_windows_is_nt();
        uint16_t winver = ardv_windows_get_version();

        LoadString(NULL, is_nt ? IDS_OLDSP : IDS_OLDOSR, format,
                   ARRAYSIZE(format));
        sprintf(message, format, cfg->name,
                ardv_windows_get_spname(winver, cfg->ossp, is_nt),
                ardv_windows_get_name(winver, is_nt));

        return die_with_rundos(message, cfg);
    }

    // check library dependencies
    if (!complete && (0 != ard_check_dependencies(cfg)))
    {
        char format[ARDC_LENGTH_MID] = "";

        ardc_dependency *dep = ard_check_sources(cfg);
        ardc_source    **sources;

        if (dep)
        {

            LoadString(NULL, IDS_NODLL, format, ARRAYSIZE(format));
            sprintf(message, format, cfg->name, dep->name, HIBYTE(dep->version),
                    LOBYTE(dep->version));

            return die_with_rundos(message, cfg);
        }

        if (NULL == (sources = ard_get_sources(cfg)))
        {
            LoadString(NULL, IDS_EBADDEPS, message, ARRAYSIZE(message));
            return die_with_rundos(message, cfg);
        }

        arda_select(cfg, sources);
        frame_ = arda_select_get_window();
    }
    else
    {
        arda_run(cfg);
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        if (!msg.hwnd || (NULL == frame_) || !IsDialogMessage(frame_, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (0 > arda_instredist_handle())
        {
            PostQuitMessage(1);
        }
    }

    arda_select_cleanup();
    ardc_cleanup();
    return msg.wParam;
}
