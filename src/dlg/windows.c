#include <windows.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <dlg.h>
#include <fmt/utf8.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <platform/windows.h>

#define ID_EDITBOX 150
#define ID_TEXT    200

static HGLOBAL           _hgbl = NULL;
static NONCLIENTMETRICSW _nclm = {0};

static HHOOK   _hook = NULL;
static WNDPROC _prev_wnd_proc = NULL;

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
    LPWSTR wtitle = (LPWSTR)alloca(title_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, title_length);

    int message_length = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
    LPWSTR wmessage = (LPWSTR)alloca(message_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, message, -1, wmessage, message_length);

    pal_disable_mouse();

    _dlg = HWND_MESSAGE;
    _hook = SetWindowsHookExW(WH_CALLWNDPROC, _hook_proc, NULL,
                              GetCurrentThreadId());
    MessageBoxW(windows_get_hwnd(), wmessage, wtitle,
                MB_OK | MB_ICONEXCLAMATION);
    _value = DLG_OK;
    _dlg = NULL;

    return true;
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

        // Get text position
        RECT text_rect;
        GetWindowRect(GetDlgItem(dlg, ID_TEXT), &text_rect);
        MapWindowPoints(HWND_DESKTOP, dlg, (LPPOINT)&text_rect, 2);
        LONG padding_x = text_rect.left;
        LONG padding_y = text_rect.top;
        text_rect.right = text_rect.left + (window_width / 2) - padding_x;

        // Get edit box position
        RECT edit_rect;
        GetClientRect(GetDlgItem(dlg, ID_EDITBOX), &edit_rect);

        // Get button position
        RECT button_rect;
        GetClientRect(GetDlgItem(dlg, IDOK), &button_rect);

        // Measure text
        HDC   dc = GetDC(dlg);
        HFONT prev_font =
            SelectObject(dc, (HFONT)SendMessageW(dlg, WM_GETFONT, 0, 0));
        DrawTextW(dc, (LPCWSTR)lparam, -1, &text_rect,
                  DT_LEFT | DT_EXPANDTABS | DT_WORDBREAK | DT_CALCRECT |
                      DT_NOPREFIX);
        SelectObject(dc, prev_font);
        ReleaseDC(dlg, dc);

        // Position everything
        LONG cl_height = padding_y;

        LONG text_width = text_rect.right - text_rect.left;
        LONG text_height = text_rect.bottom - text_rect.top;
        cl_height += text_height + padding_y;

        LONG edit_height = edit_rect.bottom - edit_rect.top;
        LONG edit_width = max(text_width, edit_rect.right - edit_rect.left);
        LONG edit_top = cl_height;
        cl_height += edit_height + 2 * padding_y;

        LONG cl_width = padding_x + edit_width + padding_x;

        LONG button_height = button_rect.bottom - button_rect.top;
        LONG button_width = button_rect.right - button_rect.left;
        LONG button_top = cl_height;
        LONG button_left = cl_width - button_width - padding_x;
        cl_height += button_height + padding_y;

        SetWindowPos(
            dlg, 0,
            wnd_rect.left + (window_width - cl_width - border_width) / 2,
            wnd_rect.top + (window_height - cl_height - border_height) / 2,
            cl_width + border_width, cl_height + border_height,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

        SetWindowPos(GetDlgItem(dlg, ID_TEXT), 0, padding_x, padding_y,
                     text_width, text_height,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

        SetWindowPos(GetDlgItem(dlg, ID_EDITBOX), 0, padding_x, edit_top,
                     edit_width, edit_height,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
        SendMessageW(GetDlgItem(dlg, ID_EDITBOX), EM_LIMITTEXT, _size, 0);

        SetWindowPos(GetDlgItem(dlg, IDOK), 0, button_left, button_top,
                     button_width, button_height,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

        // Activate edit box
        SetFocus(GetDlgItem(dlg, ID_EDITBOX));

        return TRUE;
    }

    case WM_COMMAND: {
        if ((EN_CHANGE == HIWORD(wparam)) && (ID_EDITBOX == LOWORD(wparam)))
        {
            if (!_validator)
            {
                return TRUE;
            }

            HWND   edit_box = GetDlgItem(dlg, ID_EDITBOX);
            int    length = GetWindowTextLengthW(edit_box);
            LPWSTR text = (LPWSTR)alloca((length + 1) * sizeof(WCHAR));
            GetWindowTextW(edit_box, text, length + 1);

            length =
                WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
            LPSTR atext = (LPSTR)alloca(length);
            WideCharToMultiByte(CP_UTF8, 0, text, -1, atext, length, NULL,
                                NULL);
            EnableWindow(GetDlgItem(dlg, IDOK), _validator(atext));
            return TRUE;
        }

        switch (LOWORD(wparam))
        {
        case IDOK: {
            HWND   edit_box = GetDlgItem(dlg, ID_EDITBOX);
            int    length = GetWindowTextLengthW(edit_box);
            LPWSTR text = (LPWSTR)alloca((length + 1) * sizeof(WCHAR));
            GetWindowTextW(edit_box, text, length + 1);
            WideCharToMultiByte(CP_UTF8, 0, text, -1, _buffer, _size, NULL,
                                NULL);
            DestroyWindow(dlg);
            _value = length;
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
    int dpi = GetDeviceCaps(windows_get_dc(), LOGPIXELSY);

    // Dialog box
    LPDLGTEMPLATEW dt = (LPDLGTEMPLATEW)GlobalLock(_hgbl);
    ZeroMemory(dt, 1024);
    dt->style = WS_VISIBLE | WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME |
                WS_CAPTION | DS_SETFONT;
    dt->cdit = 3;

    // no menu
    LPWORD menu = (LPWORD)(dt + 1);
    menu[0] = 0;

    // default class
    LPWORD class = (LPWORD)(menu + 1);
    class[0] = 0;

    // caption
    LPWSTR caption = (LPWSTR)(class + 1);
    int    caption_length = wcslen(title) + 1;
    wcscpy(caption, title);

    // font size
    LPWORD font_size = (LPWORD)(caption + caption_length);
    *font_size = MulDiv(_nclm.lfMessageFont.lfHeight, 96, dpi);

    // font face
    LPWSTR font_face = (LPWSTR)(font_size + 1);
    wcscpy(font_face, _nclm.lfMessageFont.lfFaceName);

    // Declaration for later
    LPWORD creation_data;

    // OK button
    LPDLGITEMTEMPLATEW ok_button = (LPDLGITEMTEMPLATEW)align(
        font_face + wcslen(font_face) + 1, sizeof(DWORD));
    ok_button->cx = 50;
    ok_button->cy = 14;
    ok_button->id = IDOK;
    ok_button->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON |
                       (_validator ? WS_DISABLED : 0);

    // button class
    class = (LPWORD)(ok_button + 1);
    class[0] = 0xFFFF;
    class[1] = 0x0080;

    // caption
    caption = (LPWSTR)(class + 2);
    caption_length = MultiByteToWideChar(CP_UTF8, 0, "OK", -1, caption, 50);

    // no creation data
    creation_data = (LPWORD)(caption + caption_length);
    *creation_data = 0;

    // Edit box
    LPDLGITEMTEMPLATEW edit_box =
        (LPDLGITEMTEMPLATEW)align(creation_data + 1, sizeof(DWORD));
    edit_box->cx = (_size + 2) * 4;
    edit_box->cy = 14;
    edit_box->id = ID_EDITBOX;
    edit_box->style = WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP |
                      ES_AUTOHSCROLL | ES_UPPERCASE;

    // static class
    class = (LPWORD)(edit_box + 1);
    class[0] = 0xFFFF;
    class[1] = 0x0081;

    // caption
    caption = (LPWSTR)(class + 2);
    *caption = 0;
    caption_length = 1;

    // no creation data
    creation_data = (LPWORD)(caption + caption_length);
    *creation_data = 0;

    // Static text control
    LPDLGITEMTEMPLATEW label =
        (LPDLGITEMTEMPLATEW)align(creation_data + 1, sizeof(DWORD));
    label->x = 5;
    label->y = 5;
    label->id = ID_TEXT;
    label->style = WS_CHILD | WS_VISIBLE | SS_LEFT;

    // static class
    class = (LPWORD)(label + 1);
    class[0] = 0xFFFF;
    class[1] = 0x0082;

    // caption
    caption = (LPWSTR)(class + 2);
    caption_length = wcslen(message) + 1;
    wcscpy(caption, message);

    // no creation data
    creation_data = (LPWORD)(caption + caption_length);
    *creation_data = 0;

    HWND wnd = CreateDialogIndirectParamW(
        GetModuleHandleW(NULL), (LPDLGTEMPLATEW)_hgbl, windows_get_hwnd(),
        (DLGPROC)_dialog_proc, (LPARAM)caption);
    GlobalUnlock(_hgbl);
    return wnd;
}

static void
_free_hgbl(void)
{
    if (_hgbl)
    {
        GlobalFree(_hgbl);
    }
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

    if (!_hgbl)
    {
        _hgbl = GlobalAlloc(0, 1024);
        if (!_hgbl)
        {
            return false;
        }

        atexit(_free_hgbl);
    }

    int    title_length = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
    LPWSTR wtitle = (LPWSTR)alloca(title_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, title_length);

    int message_length = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
    LPWSTR wmessage = (LPWSTR)alloca(message_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, message, -1, wmessage, message_length);

    _buffer = buffer;
    _size = size;
    _validator = validator;

    _value = DLG_INCOMPLETE;
    windows_set_dialog(_dlg = _create_prompt(wtitle, wmessage));

    pal_disable_mouse();

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
