#define UNICODE

#include <windows.h>
#include <windowsx.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <fmt/utf8.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <platform/windows.h>

#include "../../resource.h"
#include "../ui/encui.h"
#include "resource.h"

#ifndef PSH_AEROWIZARD
#define PSH_AEROWIZARD 0x00004000
#endif

#define WIZARD97_PADDING_LEFT 21
#define WIZARD97_PADDING_TOP  0

static NONCLIENTMETRICSW _nclm = {0};
static bool              _is_vista = WINVER >= 0x0600;
static HICON             _bang = NULL;

static encui_page *_pages = NULL;
static int         _id;

static WCHAR            _brand[MAX_PATH] = L"";
static PROPSHEETPAGEW  *_psps = NULL;
static HPROPSHEETPAGE  *_hpsps = NULL;
static PROPSHEETHEADERW _psh = {sizeof(PROPSHEETHEADERW)};

static int _value;

static bool
_set_text(HWND wnd, int ids)
{
    LPWSTR text = NULL;
    int    length = LoadStringW(NULL, ids, (LPWSTR)&text, 0);
    text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
    if (NULL == text)
    {
        return false;
    }

    LoadStringW(NULL, ids, text, length + 1);
    SetWindowTextW(wnd, text);
    free(text);
    return true;
}

static bool
_check_input(HWND dlg, int page_id)
{
    HWND   edit_box;
    size_t length;
    LPWSTR text;
    LPSTR  atext;
    int    status;

    if (-ENOSYS ==
        _pages[page_id].proc(ENCUIM_CHECK, NULL, _pages[page_id].data))
    {
        return true;
    }

    edit_box = GetDlgItem(dlg, IDC_EDITBOX);
    length = GetWindowTextLengthW(edit_box);
    text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
    if (NULL == text)
    {
        return TRUE;
    }
    GetWindowTextW(edit_box, text, length + 1);

    length = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
    atext = (LPSTR)malloc(length);
    if (NULL == atext)
    {
        free(text);
        return TRUE;
    }
    WideCharToMultiByte(CP_UTF8, 0, text, -1, atext, length, NULL, NULL);
    free(text);

    status = _pages[page_id].proc(ENCUIM_CHECK, atext, _pages[page_id].data);
    free(atext);
    return 0 == status;
}

static void
_set_buttons(HWND dlg, int id, bool has_next)
{
    bool has_previous = (0 != id) && (0 != _pages[id - 1].title);
    PropSheet_SetWizButtons(GetParent(dlg), (has_previous ? PSWIZB_BACK : 0) |
                                                (has_next ? PSWIZB_NEXT : 0));
}

static INT_PTR CALLBACK
_dialog_proc(HWND dlg, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_INITDIALOG: {
        PROPSHEETPAGEW *template = (PROPSHEETPAGEW *)lparam;
        encui_page *page = _pages + template->lParam;

#if WINVER < 0x0600
        if (!_is_vista)
        {
            HWND ps = GetParent(dlg), ctl;
            RECT padding = {WIZARD97_PADDING_LEFT, 0, WIZARD97_PADDING_TOP, 0};

            RECT wnd_rect, ps_rect, cl_rect;
            LONG window_width, window_height, border_width, border_height,
                cl_width, cl_height;

            // Get window and dialog client and non-client rects
            GetWindowRect(windows_get_hwnd(), &wnd_rect);
            window_width = wnd_rect.right - wnd_rect.left;
            window_height = wnd_rect.bottom - wnd_rect.top;

            GetWindowRect(ps, &ps_rect);
            border_height = ps_rect.bottom - ps_rect.top;
            border_width = ps_rect.right - ps_rect.left;

            GetClientRect(ps, &cl_rect);
            border_height -= cl_rect.bottom - cl_rect.top;
            border_width -= cl_rect.right - cl_rect.left;

            cl_width = cl_rect.right - cl_rect.left;
            cl_height = cl_rect.bottom - cl_rect.top;

            // Center the dialog relative to the parent window
            SetWindowPos(
                ps, 0,
                wnd_rect.left + (window_width - cl_width - border_width) / 2,
                wnd_rect.top + (window_height - cl_height - border_height) / 2,
                cl_width + border_width, cl_height + border_height,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

            // Apply padding
            MapDialogRect(dlg, &padding);
            ctl = GetWindow(dlg, GW_CHILD);
            while (ctl)
            {
                RECT ctl_rect;
                GetWindowRect(ctl, &ctl_rect);
                MapWindowPoints(NULL, dlg, (LPPOINT)&ctl_rect, 2);

                SetWindowPos(ctl, NULL, ctl_rect.left + padding.left,
                             ctl_rect.top + padding.top, 0, 0,
                             SWP_NOZORDER | SWP_NOSIZE);

                ctl = GetWindow(ctl, GW_HWNDNEXT);
            }
        }
#endif // WINVER < 0x0600

        // Set the prompt's content
        _set_text(dlg, page->title);
        _set_text(GetDlgItem(dlg, IDC_TEXT), page->message);
        SetWindowTextW(GetDlgItem(dlg, IDC_ALERT), L"");
        _set_buttons(dlg, (int)template->lParam,
                     -ENOSYS == page->proc(ENCUIM_CHECK, NULL, page->data));

        return TRUE;
    }

    case WM_NOTIFY: {
        int     id = PropSheet_HwndToIndex(GetParent(dlg), dlg);
        LPNMHDR notif = (LPNMHDR)lparam;
        switch (notif->code)
        {
        case PSN_SETACTIVE: {
            _set_buttons(dlg, id, _check_input(dlg, id));
            return 0;
        }

        case PSN_WIZNEXT: {
            int    status;
            HWND   edit_box = GetDlgItem(dlg, IDC_EDITBOX);
            size_t length = GetWindowTextLengthW(edit_box);
            LPWSTR text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
            if (NULL == text)
            {
                return -1;
            }
            GetWindowTextW(edit_box, text, length + 1);
            WideCharToMultiByte(CP_UTF8, 0, text, -1, _pages[id].prompt.buffer,
                                _pages[id].prompt.capacity, NULL, NULL);
            _value = length;
            free(text);

            status = _pages[id].proc(ENCUIM_NEXT, _pages[id].prompt.buffer,
                                     _pages[id].data);
            if (0 < status)
            {
                WCHAR message[GFX_COLUMNS];
                LoadStringW(NULL, status, message, lengthof(message));
                SetWindowTextW(GetDlgItem(dlg, IDC_ALERT), message);

                Static_SetIcon(GetDlgItem(dlg, IDC_BANG), _bang);
                MessageBeep(MB_ICONEXCLAMATION);

                SetFocus(edit_box);
                return -1;
            }

            if ((0 > status) && (-ENOSYS != status))
            {
                return -1;
            }

            if (0 != _pages[id + 1].title)
            {
                _id = id + 1;
                return 0;
            }

            PropSheet_PressButton(GetParent(dlg), PSBTN_FINISH);
            SetWindowLong(dlg, DWLP_MSGRESULT, _value);
            return 0;
        }

        case PSN_QUERYCANCEL: {
            _value = 0;
            return FALSE;
        }
        }
    }

    case WM_COMMAND: {
        int id = PropSheet_HwndToIndex(GetParent(dlg), dlg);

        if ((EN_CHANGE == HIWORD(wparam)) && (IDC_EDITBOX == LOWORD(wparam)))
        {
            _set_buttons(dlg, id, _check_input(dlg, id));
            return TRUE;
        }
    }
    }

    return FALSE;
}

bool
encui_enter(encui_page *pages, int count)
{
    int i;

    _nclm.cbSize = sizeof(_nclm);
    if (!SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(_nclm), &_nclm,
                               0))
    {
        _nclm.cbSize = 0;
        return false;
    }

#if WINVER < 0x0600
    _is_vista = 6 <= LOBYTE(GetVersion());
#endif

    _psps = (PROPSHEETPAGEW *)malloc(sizeof(PROPSHEETPAGEW) * count);
    if (NULL == _psps)
    {
        return false;
    }

    _hpsps = (HPROPSHEETPAGE *)malloc(sizeof(PROPSHEETPAGEW) * count);
    if (NULL == _hpsps)
    {
        free(_psps);
        return false;
    }

    if (0 == _brand[0])
    {
        MultiByteToWideChar(CP_UTF8, 0, pal_get_version_string(), -1, _brand,
                            lengthof(_brand));
    }

    for (i = 0; i < count; i++)
    {
        if (0 == pages[i].title)
        {
            _psps[i].dwSize = 0;
            _hpsps[i] = NULL;
            continue;
        }

        _psps[i].dwSize = sizeof(PROPSHEETPAGEW);
        _psps[i].hInstance = GetModuleHandleW(NULL);
        _psps[i].dwFlags = PSP_USEHEADERTITLE | PSP_USETITLE;
        _psps[i].lParam = (LPARAM)i;
        _psps[i].pszHeaderTitle = MAKEINTRESOURCEW(pages[i].title);
        _psps[i].pszTemplate = MAKEINTRESOURCEW(IDD_PROMPT);
        _psps[i].pszTitle = _brand;
        _psps[i].pfnDlgProc = _dialog_proc;
        _hpsps[i] = CreatePropertySheetPageW((LPCPROPSHEETPAGEW)&_psps[i]);
    }

    _psh.hInstance = GetModuleHandleW(NULL);
    _psh.phpage = _hpsps;
    _psh.hwndParent = windows_get_hwnd();
    _psh.dwFlags = _is_vista ? PSH_WIZARD | PSH_AEROWIZARD | PSH_USEICONID
                             : PSH_WIZARD97 | PSH_HEADER;
    _psh.pszCaption = _brand;
    _psh.pszIcon = MAKEINTRESOURCEW(1);
    _psh.pszbmHeader = MAKEINTRESOURCEW(IDB_HEADER);
    _psh.nStartPage = 0;
    _psh.nPages = count;

    _pages = pages;
    _id = -1;

    _bang = (HICON)LoadImageW(GetModuleHandleW(L"user32.dll"),
                              MAKEINTRESOURCEW(101), IMAGE_ICON,
                              GetSystemMetrics(SM_CXSMICON),
                              GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    return true;
}

bool
encui_exit(void)
{
    if (NULL != _psps)
    {
        free(_psps);
        _psps = NULL;
    }

    if (NULL != _hpsps)
    {
        free(_hpsps);
        _hpsps = NULL;
    }

    if (NULL != _bang)
    {
        DestroyIcon(_bang);
        _bang = NULL;
    }

    return true;
}

int
encui_get_page(void)
{
    return _id;
}

bool
encui_set_page(int id)
{
    if (0 == _pages[id].title)
    {
        _id = -1;
        return true;
    }

    _value = ENCUI_INCOMPLETE;
    _psh.nStartPage = _id = id;
    PropertySheetW(&_psh);

    pal_disable_mouse();
    return true;
}

int
encui_handle(void)
{
    int value = _value;
    _value = 0;
    return value;
}
