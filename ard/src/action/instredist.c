#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>
#include <ard/ui.h>

#include "../resource.h"

static bool                has_started_ = false;
static ardc_source       **src_ = (ardc_source **)NULL;
static PROCESS_INFORMATION child_ = {0};

static void
show_error(_In_ int id, _In_z_ const char *description)
{
    char format[ARDC_LENGTH_LONG] = "";
    char message[ARDC_LENGTH_LONG] = "";
    LoadString(NULL, id, format, ARRAYSIZE(format));
    sprintf(message, format, description);
    ardui_msgbox(message, MB_ICONERROR | MB_OK);
}

int
arda_instredist(_In_ const ardc_config *cfg, _In_ ardc_source **sources)
{
    if (has_started_)
    {
        char message[ARDC_LENGTH_LONG] = "";
        LoadString(NULL, IDS_EINSTAGAIN, message, ARRAYSIZE(message));
        ardui_msgbox(message, MB_ICONWARNING | MB_OK);
        return ARDA_ERROR;
    }

    has_started_ = true;
    src_ = sources;
    return ARDA_CONTINUE;
}

int
arda_instredist_handle(void)
{
    DWORD status = 0;

    if (!has_started_ || (NULL == src_) || (NULL == *src_))
    {
        has_started_ = false;
        return ARDA_SUCCESS;
    }

    if (0 == child_.dwProcessId)
    {
        STARTUPINFO si = {sizeof(STARTUPINFO)};
        if (!CreateProcess(NULL, (*src_)->path, NULL, NULL, FALSE, 0, NULL,
                           NULL, &si, &child_))
        {
            show_error(IDS_EINSTNOF, (*src_)->description);
            has_started_ = false;
            return ARDA_ERROR;
        }
    }

    status = MsgWaitForMultipleObjects(1, &child_.hProcess, FALSE, INFINITE,
                                       QS_ALLEVENTS);
    if (WAIT_OBJECT_0 != status)
    {
        return ARDA_CONTINUE;
    }

    GetExitCodeProcess(child_.hProcess, &status);
    CloseHandle(child_.hProcess);
    CloseHandle(child_.hThread);
    ZeroMemory(&child_, sizeof(child_));

    if (0 != status)
    {
        show_error(IDS_EINSTBAD, (*src_)->description);
        has_started_ = false;
        return -status;
    }

    src_++;
    return ARDA_SUCCESS;
}
