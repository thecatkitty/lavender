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

static LPCWSTR _title = NULL;
static LPCWSTR _message = NULL;

static char           *_buffer = NULL;
static int             _size;
static encui_validator _validator = NULL;
static enc_context    *_enc;

static int _value;

bool
encui_enter(void)
{
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

    _bang = (HICON)LoadImageW(GetModuleHandleW(L"user32.dll"),
                              MAKEINTRESOURCEW(101), IMAGE_ICON,
                              GetSystemMetrics(SM_CXSMICON),
                              GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
    return true;
}

bool
encui_exit(void)
{
    if (_bang)
    {
        DestroyIcon(_bang);
    }
    return true;
}

static INT_PTR CALLBACK
_dialog_proc(HWND dlg, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_INITDIALOG: {
#if WINVER < 0x0600
        if (!_is_vista)
        {
            HWND ps = GetParent(dlg);

            // Get window and dialog client and non-client rects
            RECT wnd_rect;
            GetWindowRect(windows_get_hwnd(), &wnd_rect);
            LONG window_width = wnd_rect.right - wnd_rect.left;
            LONG window_height = wnd_rect.bottom - wnd_rect.top;

            RECT ps_rect;
            GetWindowRect(ps, &ps_rect);
            LONG border_height = ps_rect.bottom - ps_rect.top;
            LONG border_width = ps_rect.right - ps_rect.left;

            RECT cl_rect;
            GetClientRect(ps, &cl_rect);
            border_height -= cl_rect.bottom - cl_rect.top;
            border_width -= cl_rect.right - cl_rect.left;

            LONG cl_width = cl_rect.right - cl_rect.left;
            LONG cl_height = cl_rect.bottom - cl_rect.top;

            // Center the dialog relative to the parent window
            SetWindowPos(
                ps, 0,
                wnd_rect.left + (window_width - cl_width - border_width) / 2,
                wnd_rect.top + (window_height - cl_height - border_height) / 2,
                cl_width + border_width, cl_height + border_height,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

            // Apply padding
            RECT padding = {WIZARD97_PADDING_LEFT, 0, WIZARD97_PADDING_TOP, 0};
            MapDialogRect(dlg, &padding);
            HWND ctl = GetWindow(dlg, GW_CHILD);
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
        SetWindowTextW(dlg, _title);
        SetWindowTextW(GetDlgItem(dlg, IDC_TEXT), _message);
        SetWindowTextW(GetDlgItem(dlg, IDC_ALERT), L"");

        return TRUE;
    }

    case WM_NOTIFY: {
        LPNMHDR notif = (LPNMHDR)lparam;
        switch (notif->code)
        {
        case PSN_SETACTIVE: {
            PropSheet_SetWizButtons(notif->hwndFrom,
                                    _validator ? 0 : PSBTN_NEXT);
            return 0;
        }

        case PSN_WIZNEXT: {
            HWND   edit_box = GetDlgItem(dlg, IDC_EDITBOX);
            size_t length = GetWindowTextLengthW(edit_box);
            LPWSTR text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
            if (NULL == text)
            {
                return -1;
            }
            GetWindowTextW(edit_box, text, length + 1);
            WideCharToMultiByte(CP_UTF8, 0, text, -1, _buffer, _size, NULL,
                                NULL);
            _value = length;
            free(text);

            if (_enc)
            {
                int status = __enc_decrypt_content(_enc);
                if ((0 > status) && (-EACCES != status))
                {
                    return -1;
                }

                if (-EACCES == status)
                {
                    WCHAR message[GFX_COLUMNS];
                    LoadStringW(
                        GetModuleHandleW(NULL),
                        __enc_get_provider(_enc)(ENCM_GET_ERROR_STRING, _enc),
                        message, lengthof(message));
                    SetWindowTextW(GetDlgItem(dlg, IDC_ALERT), message);

                    Static_SetIcon(GetDlgItem(dlg, IDC_BANG), _bang);
                    MessageBeep(MB_ICONEXCLAMATION);

                    SetFocus(edit_box);
                    return -1;
                }
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
            PropSheet_SetWizButtons(GetParent(dlg),
                                    _validator(atext) ? PSWIZB_NEXT : 0);

            free(text);
            free(atext);
            return TRUE;
        }
    }
    }

    return FALSE;
}

bool
encui_prompt(const char     *title,
             const char     *message,
             char           *buffer,
             int             size,
             encui_validator validator,
             enc_context    *enc)
{
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

    WCHAR branding[MAX_PATH] = L"";
    MultiByteToWideChar(CP_UTF8, 0, pal_get_version_string(), -1, branding,
                        MAX_PATH);

    PROPSHEETPAGEW   psp = {.dwSize = sizeof(PROPSHEETPAGEW),
                            .hInstance = GetModuleHandleW(NULL),
                            .dwFlags = PSP_USEHEADERTITLE | PSP_USETITLE,
                            .lParam = (LPARAM)NULL,
                            .pszHeaderTitle = wtitle,
                            .pszTemplate = MAKEINTRESOURCEW(IDD_PROMPT),
                            .pszTitle = branding,
                            .pfnDlgProc = _dialog_proc};
    HPROPSHEETPAGE   hpsp = CreatePropertySheetPageW((LPCPROPSHEETPAGEW)&psp);
    PROPSHEETHEADERW psh = {.dwSize = sizeof(PROPSHEETHEADERW),
                            .hInstance = GetModuleHandleW(NULL),
                            .phpage = &hpsp,
                            .hwndParent = windows_get_hwnd(),
                            .dwFlags = _is_vista ? PSH_WIZARD | PSH_AEROWIZARD |
                                                       PSH_USEICONID
                                                 : PSH_WIZARD97 | PSH_HEADER,
                            .pszCaption = branding,
                            .pszIcon = MAKEINTRESOURCEW(1),
                            .pszbmHeader = MAKEINTRESOURCEW(IDB_HEADER),
                            .nStartPage = 0,
                            .nPages = 1};

    _title = wtitle;
    _message = wmessage;

    _buffer = buffer;
    _size = size;
    _validator = validator;
    _enc = enc;

    _value = ENCUI_INCOMPLETE;
    PropertySheetW(&psh);

    pal_disable_mouse();

    free(wtitle);
    free(wmessage);
    return true;
}

int
encui_handle(void)
{
    int value = _value;
    _value = 0;
    return value;
}
