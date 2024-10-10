#define UNICODE

#include <windows.h>
#include <windowsx.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <dlg.h>
#include <fmt/utf8.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <platform/windows.h>

#include "../resource.h"
#include "resource.h"

static NONCLIENTMETRICSW _nclm = {0};

static HHOOK   _hook = NULL;
static WNDPROC _prev_wnd_proc = NULL;

static HICON   _icon = NULL;
static LPCWSTR _title = NULL;
static LPCWSTR _message = NULL;

static char         *_buffer = NULL;
static int           _size;
static dlg_validator _validator = NULL;

static HWND _dlg;
static int  _value;

static LRESULT CALLBACK
_hook_wnd_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT rc = CallWindowProc(_prev_wnd_proc, wnd, message, wparam, lparam);

    if (message == WM_INITDIALOG)
    {
        RECT parent_rect;
        GetWindowRect(windows_get_hwnd(), &parent_rect);
        LONG parent_width = parent_rect.right - parent_rect.left;
        LONG parent_height = parent_rect.bottom - parent_rect.top;

        RECT msgbox_rect;
        GetWindowRect(wnd, &msgbox_rect);
        LONG msgbox_width = msgbox_rect.right - msgbox_rect.left;
        LONG msgbox_height = msgbox_rect.bottom - msgbox_rect.top;

        MoveWindow(wnd, parent_rect.left + (parent_width - msgbox_width) / 2,
                   parent_rect.top + (parent_height - msgbox_height) / 2,
                   msgbox_width, msgbox_height, FALSE);
        return rc;
    }

    if (message == WM_NCDESTROY)
    {
        UnhookWindowsHookEx(_hook);
    }

    return rc;
}

static LRESULT CALLBACK
_hook_proc(int code, WPARAM wparam, LPARAM lparam)
{
    if (HC_ACTION == code)
    {
        LPCWPSTRUCT cwp = (LPCWPSTRUCT)lparam;
        if (cwp->message == WM_INITDIALOG)
        {
            _prev_wnd_proc = (WNDPROC)SetWindowLongPtrW(
                cwp->hwnd, GWLP_WNDPROC, (LONG_PTR)_hook_wnd_proc);
        }
    }

    return CallNextHookEx(_hook, code, wparam, lparam);
}

bool
dlg_alert(const char *title, const char *message)
{
    if (_dlg)
    {
        return false;
    }

    int    title_length = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
    LPWSTR wtitle = (LPWSTR)malloc(title_length * sizeof(WCHAR));
    if (NULL == wtitle)
    {
        return false;
    }
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, title_length);

    int message_length = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
    LPWSTR wmessage = (LPWSTR)malloc(message_length * sizeof(WCHAR));
    if (NULL == wmessage)
    {
        free(wtitle);
        return false;
    }
    MultiByteToWideChar(CP_UTF8, 0, message, -1, wmessage, message_length);

    pal_disable_mouse();

    _dlg = HWND_MESSAGE;
    _hook = SetWindowsHookExW(WH_CALLWNDPROC, _hook_proc, NULL,
                              GetCurrentThreadId());
    MessageBoxW(windows_get_hwnd(), wmessage, wtitle,
                MB_OK | MB_ICONEXCLAMATION);
    _value = DLG_OK;
    _dlg = NULL;

    free(wtitle);
    free(wmessage);
    return true;
}

typedef HRESULT(WINAPI *pfloadiconmetric)(HINSTANCE, PCWSTR, int, HICON *);

static HICON
_load_icon(void)
{
    HMODULE comctl = LoadLibraryW(L"comctl32.dll");
    if (NULL == comctl)
    {
        return NULL;
    }

    pfloadiconmetric loadiconmetric =
        (pfloadiconmetric)GetProcAddress(comctl, "LoadIconMetric");
    if (NULL == loadiconmetric)
    {
        FreeLibrary(comctl);
        return NULL;
    }

    HICON icon = NULL;
    loadiconmetric(NULL, IDI_QUESTION, 1, &icon);
    FreeLibrary(comctl);

    return icon;
}

static BOOL CALLBACK
_dialog_proc(HWND dlg, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_INITDIALOG: {
        // Get window and dialog client and non-client rects
        RECT wnd_rect;
        GetWindowRect(windows_get_hwnd(), &wnd_rect);
        LONG window_width = wnd_rect.right - wnd_rect.left;
        LONG window_height = wnd_rect.bottom - wnd_rect.top;

        RECT dlg_rect;
        GetWindowRect(dlg, &dlg_rect);
        LONG border_height = dlg_rect.bottom - dlg_rect.top;
        LONG border_width = dlg_rect.right - dlg_rect.left;

        RECT cl_rect;
        GetClientRect(dlg, &cl_rect);
        border_height -= cl_rect.bottom - cl_rect.top;
        border_width -= cl_rect.right - cl_rect.left;

        LONG cl_width = cl_rect.right - cl_rect.left;
        LONG cl_height = cl_rect.bottom - cl_rect.top;

        SetWindowPos(
            dlg, 0,
            wnd_rect.left + (window_width - cl_width - border_width) / 2,
            wnd_rect.top + (window_height - cl_height - border_height) / 2,
            cl_width + border_width, cl_height + border_height,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

        // Set the prompt's content
        SetWindowTextW(dlg, _title);

        HICON icon = _icon = _load_icon();
        if (NULL == icon)
        {
            icon = LoadIconW(NULL, IDI_QUESTION);
        }
        Static_SetIcon(GetDlgItem(dlg, IDC_DLGICON), icon);

        SetWindowTextW(GetDlgItem(dlg, IDC_TEXT), _message);

        WCHAR msg[10];
        LoadStringW(GetModuleHandleW(NULL), IDS_OK, msg, lengthof(msg));
        SetWindowTextW(GetDlgItem(dlg, IDOK), msg);
        if (_validator)
        {
            EnableWindow(GetDlgItem(dlg, IDOK), FALSE);
        }

        LoadStringW(GetModuleHandleW(NULL), IDS_CANCEL, msg, lengthof(msg));
        SetWindowTextW(GetDlgItem(dlg, IDCANCEL), msg);

        return TRUE;
    }

    case WM_NCDESTROY: {
        if (NULL != _icon)
        {
            DestroyIcon(_icon);
            _icon = NULL;
        }

        break;
    }

    case WM_ERASEBKGND: {
        if (6 > LOBYTE(GetVersion()))
        {
            break;
        }

        // Vista-style content and action area
        HDC dc = (HDC)wparam;

        RECT cl_rect;
        GetClientRect(dlg, &cl_rect);

        RECT btn_rect;
        GetWindowRect(GetDlgItem(dlg, IDOK), &btn_rect);

        RECT content_rect = cl_rect;
        content_rect.bottom -= 2 * (btn_rect.bottom - btn_rect.top);
        FillRect(dc, &content_rect, GetSysColorBrush(COLOR_WINDOW));

        RECT action_rect = cl_rect;
        action_rect.top = content_rect.bottom;
        FillRect(dc, &action_rect, GetSysColorBrush(COLOR_3DFACE));

        return TRUE;
    }

    case WM_CTLCOLORSTATIC: {
        if (6 > LOBYTE(GetVersion()))
        {
            break;
        }

        // Use Vista-style content area
        SetBkColor((HDC)wparam, GetSysColor(COLOR_WINDOW));
        return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
    }

    case WM_COMMAND: {
        if ((EN_CHANGE == HIWORD(wparam)) && (IDC_EDITBOX == LOWORD(wparam)))
        {
            if (!_validator)
            {
                return TRUE;
            }

            HWND   edit_box = GetDlgItem(dlg, IDC_EDITBOX);
            size_t length = GetWindowTextLengthW(edit_box);
            LPWSTR text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
            if (NULL == text)
            {
                return TRUE;
            }
            GetWindowTextW(edit_box, text, length + 1);

            length =
                WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
            LPSTR atext = (LPSTR)malloc(length);
            if (NULL == atext)
            {
                free(text);
                return TRUE;
            }
            WideCharToMultiByte(CP_UTF8, 0, text, -1, atext, length, NULL,
                                NULL);
            EnableWindow(GetDlgItem(dlg, IDOK), _validator(atext));

            free(text);
            free(atext);
            return TRUE;
        }

        switch (LOWORD(wparam))
        {
        case IDOK: {
            HWND   edit_box = GetDlgItem(dlg, IDC_EDITBOX);
            size_t length = GetWindowTextLengthW(edit_box);
            LPWSTR text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
            if (NULL == text)
            {
                return TRUE;
            }
            GetWindowTextW(edit_box, text, length + 1);
            WideCharToMultiByte(CP_UTF8, 0, text, -1, _buffer, _size, NULL,
                                NULL);
            DestroyWindow(dlg);
            _value = length;
            free(text);
            return TRUE;
        }

        case IDCANCEL:
            DestroyWindow(dlg);
            _value = 0;
            return TRUE;

        default:
            return FALSE;
        }
    }
    }

    return FALSE;
}

static HWND
_create_prompt(LPCWSTR title, LPCWSTR message)
{
    _title = title;
    _message = message;

    HWND wnd = CreateDialogParamW(
        GetModuleHandleW(NULL), MAKEINTRESOURCEW(IDD_PROMPT),
        windows_get_hwnd(), (DLGPROC)_dialog_proc, (LPARAM)NULL);

    int   dpi = GetDeviceCaps(windows_get_dc(), LOGPIXELSY);
    HFONT font =
        CreateFontW(MulDiv(_nclm.lfMessageFont.lfHeight, 96, dpi), 0, 0, 0,
                    FW_REGULAR, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                    FF_DONTCARE, _nclm.lfMessageFont.lfFaceName);
    SendMessageW(wnd, WM_SETFONT, (WPARAM)font, TRUE);

    return wnd;
}

bool
dlg_prompt(const char   *title,
           const char   *message,
           char         *buffer,
           int           size,
           dlg_validator validator)
{
    if (_dlg)
    {
        return false;
    }

    if (0 == _nclm.cbSize)
    {
        _nclm.cbSize = sizeof(_nclm);
        if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(_nclm),
                                   &_nclm, 0))
        {
            _nclm.cbSize = 0;
            return false;
        }
    }

    int    title_length = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
    LPWSTR wtitle = (LPWSTR)malloc(title_length * sizeof(WCHAR));
    if (NULL == wtitle)
    {
        return false;
    }
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, title_length);

    int message_length = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
    LPWSTR wmessage = (LPWSTR)malloc(message_length * sizeof(WCHAR));
    if (NULL == wmessage)
    {
        free(wtitle);
        return false;
    }
    MultiByteToWideChar(CP_UTF8, 0, message, -1, wmessage, message_length);

    _buffer = buffer;
    _size = size;
    _validator = validator;

    _value = DLG_INCOMPLETE;
    windows_set_dialog(_dlg = _create_prompt(wtitle, wmessage));

    pal_disable_mouse();

    free(wtitle);
    free(wmessage);
    return true;
}

int
dlg_handle(void)
{
    if (NULL == _dlg)
    {
        return 0;
    }

    if (DLG_INCOMPLETE != _value)
    {
        windows_set_dialog(_dlg = NULL);
    }

    return _value;
}
