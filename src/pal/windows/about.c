#define UNICODE
#include <windows.h>
#include <windowsx.h>

#include <arch/windows.h>
#include <pal.h>

#include "../../resource.h"
#include "impl.h"

static HHOOK   hook_ = NULL;
static WNDPROC prev_wndproc_ = NULL;

static BOOL CALLBACK
enum_child_proc(HWND wnd, LPARAM lparam)
{
    LONG_PTR style;

    wchar_t wndc_name[256];
    GetClassNameW(wnd, wndc_name, lengthof(wndc_name));
    if (0 != wcsicmp(wndc_name, L"Static"))
    {
        return TRUE;
    }

    style = GetWindowLongW(wnd, GWL_STYLE);
    if (SS_ICON & style)
    {
        *(HWND *)lparam = wnd;
        return FALSE;
    }

    return TRUE;
}

static LRESULT CALLBACK
hook_wndproc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT rc = CallWindowProc(prev_wndproc_, wnd, message, wparam, lparam);

    if (message == WM_INITDIALOG)
    {
        HWND icon_wnd = NULL;
        RECT parent_rect, msgbox_rect;
        LONG parent_width, parent_height, msgbox_width, msgbox_height;

        if (NULL != windows_wnd)
        {
            GetWindowRect(windows_wnd, &parent_rect);
            parent_width = parent_rect.right - parent_rect.left;
            parent_height = parent_rect.bottom - parent_rect.top;

            GetWindowRect(wnd, &msgbox_rect);
            msgbox_width = msgbox_rect.right - msgbox_rect.left;
            msgbox_height = msgbox_rect.bottom - msgbox_rect.top;

            MoveWindow(wnd,
                       parent_rect.left + (parent_width - msgbox_width) / 2,
                       parent_rect.top + (parent_height - msgbox_height) / 2,
                       msgbox_width, msgbox_height, FALSE);
        }

        EnumChildWindows(wnd, enum_child_proc, (LPARAM)&icon_wnd);
        if (NULL != icon_wnd)
        {
            HICON icon = LoadIconW(windows_instance, MAKEINTRESOURCEW(2));
            Static_SetIcon(icon_wnd, icon);
        }

        return rc;
    }

    if (message == WM_NCDESTROY)
    {
        UnhookWindowsHookEx(hook_);
    }

    return rc;
}

static LRESULT CALLBACK
hook_proc(int code, WPARAM wparam, LPARAM lparam)
{
    if (HC_ACTION == code)
    {
        LPCWPSTRUCT cwp = (LPCWPSTRUCT)lparam;
        if (cwp->message == WM_INITDIALOG)
        {
            prev_wndproc_ = (WNDPROC)SetWindowLongPtrW(cwp->hwnd, GWLP_WNDPROC,
                                                       (LONG_PTR)hook_wndproc);
        }
    }

    return CallNextHookEx(hook_, code, wparam, lparam);
}

void
windows_about(const wchar_t *title, const wchar_t *text)
{
    const char *version = pal_get_version_string();
    wchar_t     message[1024];
    wchar_t     part[MAX_PATH];

    MultiByteToWideChar(CP_UTF8, 0, version, -1, message, lengthof(message));
    windows_append(message, L"\n", lengthof(message));
    LoadStringW(windows_instance, IDS_DESCRIPTION, part, lengthof(part));
    windows_append(message, part, lengthof(message));
    windows_append(message, L"\n\n", lengthof(message));

    if (NULL != text)
    {
        windows_append(message, text, lengthof(message));
    }

    LoadStringW(windows_instance, IDS_COPYRIGHT, part, lengthof(part));
    windows_append(message, part, lengthof(message));
    windows_append(message, L"\n\nhttps://celones.pl/lavender",
                   lengthof(message));

    hook_ = SetWindowsHookExW(WH_CALLWNDPROC, hook_proc, NULL,
                              GetCurrentThreadId());

    if (NULL == title)
    {
        LoadStringW(windows_instance, IDS_ABOUT_LONG, part, lengthof(part));
        title = part;
    }
    MessageBoxW(windows_wnd, message, title, MB_ICONQUESTION);
}
