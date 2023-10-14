#include <stdlib.h>
#include <string.h>

#include <dlg.h>
#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <platform/windows.h>

enum
{
    STATE_NONE,
    STATE_ALERT,
    STATE_PROMPT
};

static gfx_dimensions _screen = {0, 0};
static int            _state = STATE_NONE;

static HGLOBAL           _hgbl = NULL;
static NONCLIENTMETRICSW _nclm = {0};

static void
_draw_background(void)
{
    if (!_screen.width)
    {
        gfx_get_screen_dimensions(&_screen);
    }

    gfx_fill_rectangle(&_screen, 0, 0, GFX_COLOR_GRAY);

    gfx_dimensions bar = {_screen.width, 9};
    gfx_fill_rectangle(&bar, 0, 0, GFX_COLOR_WHITE);

    gfx_dimensions hline = {_screen.width - 1, 1};
    gfx_draw_line(&hline, 0, bar.height, GFX_COLOR_BLACK);
    gfx_draw_text(pal_get_version_string(), 1, 0);

    gfx_fill_rectangle(&bar, 0, _screen.height - bar.height, GFX_COLOR_BLACK);
    gfx_draw_text("(C) 2021-2023", 1, 24);
    gfx_draw_text("https://github.com/thecatkitty/lavender/", 39, 24);
}

static struct
{
    LPWSTR title;
    LPWSTR message;
    HANDLE thread;
    int    value;
} _alert_data;

static DWORD WINAPI
_alert_routine(LPVOID lpparam)
{
    MessageBoxW(windows_get_hwnd(), _alert_data.message, _alert_data.title,
                MB_OK);
    _alert_data.value = DLG_OK;

    free(_alert_data.title);
    free(_alert_data.message);
    return 0;
}

static int
_handle_alert(void)
{
    if (DLG_INCOMPLETE != _alert_data.value)
    {
        WaitForSingleObject(_alert_data.thread, INFINITE);
        CloseHandle(_alert_data.thread);
        _alert_data.thread = NULL;
        _state = STATE_NONE;
    }

    return _alert_data.value;
}

bool
dlg_alert(const char *title, const char *message)
{
    if (STATE_NONE != _state)
    {
        return false;
    }

    int title_length = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
    _alert_data.title = (LPWSTR)malloc(title_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, title, -1, _alert_data.title, title_length);

    int message_length = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
    _alert_data.message = (LPWSTR)malloc(message_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, message, -1, _alert_data.message,
                        message_length);

    _alert_data.thread = CreateThread(NULL, 0, _alert_routine, NULL, 0, NULL);
    _alert_data.value = DLG_INCOMPLETE;

    _draw_background();

    _state = STATE_ALERT;
    return true;
}

static struct
{
    LPWSTR        title;
    LPWSTR        message;
    HANDLE        thread;
    char         *buffer;
    int           size;
    dlg_validator validator;
    int           value;
} _prompt_data;

static int
_handle_prompt(void)
{
    if (DLG_INCOMPLETE != _prompt_data.value)
    {
        if (WAIT_TIMEOUT == WaitForSingleObject(_prompt_data.thread, 1))
        {
            return DLG_INCOMPLETE;
        }

        CloseHandle(_prompt_data.thread);
        _prompt_data.thread = NULL;
        _state = STATE_NONE;
    }

    return _prompt_data.value;
}

#define ID_EDITBOX 150
#define ID_TEXT    200

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
        SendMessageW(GetDlgItem(dlg, ID_EDITBOX), EM_LIMITTEXT,
                     _prompt_data.size, 0);

        SetWindowPos(GetDlgItem(dlg, IDOK), 0, button_left, button_top,
                     button_width, button_height,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

        // Activate edit box
        SetFocus(GetDlgItem(dlg, ID_EDITBOX));

        return FALSE;
    }

    case WM_COMMAND:
        if ((EN_CHANGE == HIWORD(wparam)) && (ID_EDITBOX == LOWORD(wparam)))
        {
            if (!_prompt_data.validator)
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
            EnableWindow(GetDlgItem(dlg, IDOK), _prompt_data.validator(atext));
            return TRUE;
        }

        switch (LOWORD(wparam))
        {
        case IDOK: {
            HWND   edit_box = GetDlgItem(dlg, ID_EDITBOX);
            int    length = GetWindowTextLengthW(edit_box);
            LPWSTR text = (LPWSTR)alloca((length + 1) * sizeof(WCHAR));
            GetWindowTextW(edit_box, text, length + 1);
            WideCharToMultiByte(CP_UTF8, 0, text, -1, _prompt_data.buffer,
                                _prompt_data.size, NULL, NULL);
            EndDialog(dlg, wparam);
            _prompt_data.value = length;
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(dlg, wparam);
            _prompt_data.value = 0;
            // Fall through

        default:
            return TRUE;
        }
    }
    return FALSE;
}

static DWORD WINAPI
_prompt_routine(LPVOID lpparam)
{
    // Dialog box
    LPDLGTEMPLATE dt = (LPDLGTEMPLATE)GlobalLock(_hgbl);
    ZeroMemory(dt, 1024);
    dt->style = WS_POPUP | WS_BORDER | WS_SYSMENU | DS_MODALFRAME | WS_CAPTION |
                DS_SETFONT;
    dt->cdit = 3;

    // no menu
    LPWORD menu = (LPWORD)(dt + 1);
    menu[0] = 0;

    // default class
    LPWORD class = (LPWORD)(menu + 1);
    class[0] = 0;

    // caption
    LPWSTR caption = (LPWSTR)(class + 1);
    int    caption_length = wcslen(_prompt_data.title) + 1;
    wcscpy(caption, _prompt_data.title);

    // font size
    LPWORD font_size = (LPWORD)(caption + caption_length);
    *font_size = _nclm.lfMessageFont.lfHeight;

    // font face
    LPWSTR font_face = (LPWSTR)(font_size + 1);
    wcscpy(font_face, _nclm.lfMessageFont.lfFaceName);

    // Declaration for later
    LPWORD creation_data;

    // OK button
    LPDLGITEMTEMPLATE ok_button = (LPDLGITEMTEMPLATE)align(
        font_face + wcslen(font_face) + 1, sizeof(DWORD));
    ok_button->cx = 50;
    ok_button->cy = 14;
    ok_button->id = IDOK;
    ok_button->style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON |
                       (_prompt_data.validator ? WS_DISABLED : 0);

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
    LPDLGITEMTEMPLATE edit_box =
        (LPDLGITEMTEMPLATE)align(creation_data + 1, sizeof(DWORD));
    edit_box->cx = (_prompt_data.size + 2) * 4;
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
    LPDLGITEMTEMPLATE label =
        (LPDLGITEMTEMPLATE)align(creation_data + 1, sizeof(DWORD));
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
    caption_length = wcslen(_prompt_data.message) + 1;
    wcscpy(caption, _prompt_data.message);

    // no creation data
    creation_data = (LPWORD)(caption + caption_length);
    *creation_data = 0;

    GlobalUnlock(_hgbl);
    DialogBoxIndirectParamW(GetModuleHandleW(NULL), (LPDLGTEMPLATE)_hgbl,
                            windows_get_hwnd(), (DLGPROC)_dialog_proc,
                            (LPARAM)caption);

    free(_prompt_data.title);
    free(_prompt_data.message);
    return 0;
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
    if (STATE_NONE != _state)
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

    int title_length = MultiByteToWideChar(CP_UTF8, 0, title, -1, NULL, 0);
    _prompt_data.title = (LPWSTR)malloc(title_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, title, -1, _prompt_data.title,
                        title_length);

    int message_length = MultiByteToWideChar(CP_UTF8, 0, message, -1, NULL, 0);
    _prompt_data.message = (LPWSTR)malloc(message_length * sizeof(WCHAR));
    MultiByteToWideChar(CP_UTF8, 0, message, -1, _prompt_data.message,
                        message_length);

    _prompt_data.buffer = buffer;
    _prompt_data.size = size;
    _prompt_data.validator = validator;

    _prompt_data.thread = CreateThread(NULL, 0, _prompt_routine, NULL, 0, NULL);
    _prompt_data.value = DLG_INCOMPLETE;

    _draw_background();

    _state = STATE_PROMPT;
    return true;
}

int
dlg_handle(void)
{
    switch (_state & 0xFF)
    {
    case STATE_ALERT:
        return _handle_alert();

    case STATE_PROMPT:
        return _handle_prompt();

    default:
        return 0;
    }
}
