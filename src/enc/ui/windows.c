#define UNICODE

// clang-format off
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
// clang-format on

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <arch/windows.h>
#include <fmt/utf8.h>
#include <pal.h>

#include "../../resource.h"
#include "../ui/encui.h"
#include "resource.h"

typedef HRESULT(STDAPICALLTYPE *pf_shgetstockiconinfo)(SHSTOCKICONID,
                                                       UINT,
                                                       SHSTOCKICONINFO *);

#ifndef PSH_AEROWIZARD
#define PSH_AEROWIZARD 0x00004000
#endif

#ifndef BS_COMMANDLINK
#define BS_COMMANDLINK    0x0000000EL
#define BS_DEFCOMMANDLINK 0x0000000FL

#define BCM_FIRST        0x1600
#define BCM_GETIDEALSIZE (BCM_FIRST + 0x0001)
#define BCM_SETNOTE      (BCM_FIRST + 0x0009)
#endif

#ifndef LWS_RIGHT
#define LWS_RIGHT 0x20
#endif

#define WIZARD97_PADDING_LEFT 21
#define WIZARD97_PADDING_TOP  0

#define CPX_CTLID(i) (0x100 + (i))

#define WM_ENCUI_NOTIFY (WM_USER + 1)

#define IDT_ENTERED 1

static NONCLIENTMETRICSW _nclm = {0};
static HICON             _bang = NULL;

static encui_page *_pages = NULL;
static int         _id;

static WCHAR            _brand[MAX_PATH] = L"";
static PROPSHEETPAGEW  *_psps = NULL;
static HPROPSHEETPAGE  *_hpsps = NULL;
static PROPSHEETHEADERW _psh = {sizeof(PROPSHEETHEADERW)};
static HWND             _wnd = NULL;
static HWND             _active_dlg = NULL;

static HFONT _font = NULL;
static HFONT _prev_font = NULL;
static bool  _allocated_font = false;
static LONG  _avg_width = 1, _prev_avg_width = 1;

static int _value;

static int
_set_text(HWND wnd, uintptr_t ids, bool measure)
{
    int    height = 0;
    LPWSTR text = NULL;
    int    length = 0;

    if (0x10000 > ids)
    {
        length = LoadStringW(NULL, ids, (LPWSTR)&text, 0);
        if (windows_is_less_than_2000() && (0 >= length))
        {
            length = 4096;
        }
    }
    else
    {
        length = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)ids, -1, NULL, 0);
    }

    text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
    if (NULL == text)
    {
        return height;
    }

    if (0x10000 > ids)
    {
        LoadStringW(NULL, ids, text, length + 1);
    }
    else
    {
        MultiByteToWideChar(CP_UTF8, 0, (LPCCH)ids, -1, text, length + 1);
    }

    SetWindowTextW(wnd, text);

    if (measure)
    {
        HDC  dc = GetDC(wnd);
        RECT rect;
        GetClientRect(wnd, &rect);
        SelectObject(dc, _font);
        DrawTextW(dc, text, -1, &rect, DT_CALCRECT | DT_WORDBREAK);
        ReleaseDC(wnd, dc);
        height = rect.bottom;

        GetWindowRect(wnd, &rect);
        MapWindowPoints(NULL, GetParent(wnd), (LPPOINT)&rect, 2);
        SetWindowPos(wnd, NULL, rect.left, rect.top, rect.right - rect.left,
                     height, SWP_NOZORDER | SWP_NOACTIVATE);
    }

    free(text);
    return height;
}

static int
_set_plain_text(HWND wnd, uintptr_t ids)
{
    int   height = 0;
    int   length;
    LPSTR text = NULL;
    LPSTR src = NULL, dst = NULL;
    bool  skipping = false;

    if (0x10000 > ids)
    {
        LPWSTR wtext;
        int    wlength = LoadStringW(NULL, ids, (LPWSTR)&wtext, 0);
        length = WideCharToMultiByte(CP_UTF8, 0, wtext, wlength, NULL, 0, NULL,
                                     NULL);
    }
    else
    {
        length = strlen((char *)ids);
    }

    text = malloc(length + 1);
    if (NULL == text)
    {
        return height;
    }

    if (0x10000 > ids)
    {
        LPWSTR wtext;
        int    wlength = LoadStringW(NULL, ids, (LPWSTR)&wtext, 0);
        WideCharToMultiByte(CP_UTF8, 0, wtext, wlength, text, length + 1, NULL,
                            NULL);
        text[length] = 0;
        src = text;
    }
    else
    {
        src = (char *)ids;
    }

    dst = text;
    while (*src)
    {
        if ('<' == *src)
        {
            skipping = true;
        }

        if (!skipping)
        {
            *dst = *src;
            dst++;
        }

        if ('>' == *src)
        {
            skipping = false;
        }

        src++;
    }
    *dst = 0;

    height = _set_text(wnd, (uintptr_t)text, true);

    free(text);
    return height;
}

static int
_get_separator_height(HWND dlg, HFONT font)
{
    HDC  dc = GetDC(dlg);
    RECT line_rect = {0, 0, 0, 0};
    SelectObject(dc, font);
    DrawTextW(dc, L"lightyear", -1, &line_rect, DT_CALCRECT);
    ReleaseDC(dlg, dc);
    return line_rect.bottom;
}

static bool
_check_input(HWND dlg, int page_id)
{
    HWND   edit_box;
    size_t length;
    LPWSTR text;
    LPSTR  atext;
    int    status;

    if (-ENOSYS == encui_check_page(_pages + page_id, NULL))
    {
        return true;
    }

    edit_box = GetDlgItem(dlg, IDC_EDITBOX);
    if (NULL == edit_box)
    {
        return true;
    }

    length = GetWindowTextLengthW(edit_box);
    text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
    if (NULL == text)
    {
        return true;
    }
    GetWindowTextW(edit_box, text, length + 1);

    length = WideCharToMultiByte(CP_UTF8, 0, text, -1, NULL, 0, NULL, NULL);
    atext = (LPSTR)malloc(length);
    if (NULL == atext)
    {
        free(text);
        return true;
    }
    WideCharToMultiByte(CP_UTF8, 0, text, -1, atext, length, NULL, NULL);
    free(text);

    status = encui_check_page(_pages + page_id, atext);
    free(atext);
    return 0 == status;
}

static bool
_has_syslink(void)
{
    INITCOMMONCONTROLSEX icc = {sizeof(INITCOMMONCONTROLSEX), ICC_LINK_CLASS};
    return InitCommonControlsEx(&icc);
}

static void
_get_scaled_dimensions(gfx_bitmap *bm, int *width, int *height)
{
    HDC   dc = GetDC(NULL);
    float scale = (float)GetDeviceCaps(dc, LOGPIXELSX) / 96.f;
    ReleaseDC(NULL, dc);

    *width = bm->width * scale;
    *height = bm->height * scale;
}

static void
_set_scaled_bitmap(HWND ctl, gfx_bitmap *bm, int new_width, int new_height)
{
    HBITMAP bmp, old_src_bmp, old_dst_bmp, new_bmp;
    HDC     src_dc, dst_dc;

    src_dc = CreateCompatibleDC(NULL);
    dst_dc = CreateCompatibleDC(NULL);

    bmp = windows_create_dib(src_dc, bm);
    old_src_bmp = (HBITMAP)SelectObject(src_dc, bmp);
    new_bmp = CreateCompatibleBitmap(src_dc, new_width, new_height);
    old_dst_bmp = (HBITMAP)SelectObject(dst_dc, new_bmp);

    StretchBlt(dst_dc, 0, 0, new_width, new_height, src_dc, 0, 0, bm->width,
               bm->height, SRCCOPY);

    SelectObject(src_dc, old_src_bmp);
    SelectObject(dst_dc, old_dst_bmp);
    DeleteObject(bmp);
    DeleteDC(src_dc);
    DeleteDC(dst_dc);

    SendMessageW(ctl, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)new_bmp);
}

static void
_create_controls(HWND dlg, encui_page *page)
{
    RECT rect;
    int  cx, cy, my, i;
    bool has_checkbox = false, has_textbox = false, has_options = false;

    my = _get_separator_height(dlg, _font);
    SetWindowTextW(GetDlgItem(dlg, IDC_TEXT), L"");
    SetWindowTextW(GetDlgItem(dlg, IDC_ALERT), L"");

    GetWindowRect(GetDlgItem(dlg, IDC_TEXT), &rect);
    ScreenToClient(dlg, (POINT *)&rect.left);
    ScreenToClient(dlg, (POINT *)&rect.right);
    cx = rect.left;
    cy = rect.top;

    for (i = 0; i < page->length; i++)
    {
        encui_field *field = &page->fields[i];

        if (ENCUIFT_SEPARATOR == field->type)
        {
            cy += my * field->data;
        }

        if (ENCUIFT_LABEL == field->type)
        {
            HWND ctl;

            if ((ENCUIFF_BODY == (ENCUIFF_POSITION & field->flags)) ||
                !(windows_is_at_least_vista() || _has_syslink()))
            {
                DWORD style =
                    WS_VISIBLE | WS_CHILD |
                    (ENCUIFF_CENTER == (ENCUIFF_ALIGN & field->flags)
                         ? SS_CENTER
                         : 0) |
                    (ENCUIFF_RIGHT == (ENCUIFF_ALIGN & field->flags) ? SS_RIGHT
                                                                     : 0);
                ctl = CreateWindowW(L"STATIC", L"", style, cx, cy,
                                    rect.right - rect.left, 64, dlg,
                                    (HMENU)(UINT_PTR)CPX_CTLID(i),
                                    GetModuleHandleW(NULL), NULL);
            }
            else
            {
                DWORD style =
                    WS_VISIBLE | WS_CHILD |
                    (ENCUIFF_RIGHT == (ENCUIFF_ALIGN & field->flags) ? LWS_RIGHT
                                                                     : 0);
                ctl = CreateWindowW(WC_LINK, L"", style, cx, cy,
                                    rect.right - rect.left, 64, dlg,
                                    (HMENU)(UINT_PTR)CPX_CTLID(i),
                                    GetModuleHandleW(NULL), NULL);
            }

            SendMessageW(ctl, WM_SETFONT, (WPARAM)_font, TRUE);
            if (ENCUIFF_FOOTER == (ENCUIFF_POSITION & field->flags))
            {
                RECT dlg_rect;
                int  height = (windows_is_at_least_vista() || _has_syslink())
                                  ? _set_text(ctl, field->data, true)
                                  : _set_plain_text(ctl, field->data);
                GetWindowRect(dlg, &dlg_rect);
                MoveWindow(ctl, cx, dlg_rect.bottom - dlg_rect.top - height,
                           rect.right - rect.left, height, TRUE);
            }
            else
            {
                cy += _set_text(ctl, field->data, true) + my;
            }
        }

        if (ENCUIFT_TEXTBOX == field->type)
        {
            HWND box, ctl;
            RECT box_rect, ctl_rect;

            if (has_textbox)
            {
                continue;
            }

            has_textbox = true;

            box = GetDlgItem(dlg, IDC_EDITBOX);
            GetWindowRect(box, &box_rect);
            MoveWindow(
                box, cx, cy,
                min(box_rect.right - box_rect.left, rect.right - rect.left),
                box_rect.bottom - box_rect.top, TRUE);

            ctl = GetDlgItem(dlg, IDC_BANG);
            GetWindowRect(ctl, &ctl_rect);
            MoveWindow(ctl, cx + ctl_rect.left - box_rect.left,
                       cy + ctl_rect.top - box_rect.top,
                       ctl_rect.right - ctl_rect.left,
                       ctl_rect.bottom - ctl_rect.top, TRUE);

            ctl = GetDlgItem(dlg, IDC_ALERT);
            GetWindowRect(ctl, &ctl_rect);
            MoveWindow(ctl, cx + ctl_rect.left - box_rect.left,
                       cy + ctl_rect.top - box_rect.top,
                       ctl_rect.right - ctl_rect.left,
                       ctl_rect.bottom - ctl_rect.top, TRUE);
            SendMessageW(ctl, WM_SETFONT, (WPARAM)_font, TRUE);

            cy += ctl_rect.bottom - box_rect.top + my;
        }

        if (ENCUIFT_CHECKBOX == field->type)
        {
            if (has_checkbox)
            {
                continue;
            }

            has_checkbox = true;
            _set_text(GetDlgItem(dlg, IDC_CHECK), field->data, false);
            if (ENCUIFF_CHECKED & field->flags)
            {
                Button_SetCheck(GetDlgItem(dlg, IDC_CHECK), BST_CHECKED);
            }
        }

        if (ENCUIFT_OPTION == field->type)
        {
            if (!windows_is_at_least_vista())
            {
                DWORD style = WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON |
                              (has_options ? 0 : WS_GROUP);
                HWND ctl = CreateWindowW(L"BUTTON", L"", style, cx, cy,
                                         rect.right - rect.left, 64, dlg,
                                         (HMENU)(UINT_PTR)CPX_CTLID(i),
                                         GetModuleHandleW(NULL), NULL);

                has_options = true;
                SendMessageW(ctl, WM_SETFONT, (WPARAM)_font, TRUE);
                cy += _set_text(ctl, field->data, true) + my;
                if (ENCUIFF_CHECKED & field->flags)
                {
                    Button_SetCheck(ctl, BST_CHECKED);
                }
            }
            else
            {
                DWORD style =
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD |
                    ((ENCUIFF_CHECKED & field->flags) ? BS_DEFCOMMANDLINK
                                                      : BS_COMMANDLINK);
                HWND ctl = CreateWindowW(L"BUTTON", L"", style, cx, cy,
                                         rect.right - rect.left, 128, dlg,
                                         (HMENU)(UINT_PTR)CPX_CTLID(i),
                                         GetModuleHandleW(NULL), NULL);
                SIZE ideal_size = {rect.right - rect.left};

                if (0 == (ENCUIFF_DYNAMIC & field->flags))
                {
                    WCHAR buff[MAX_PATH];

                    if (0 == LoadStringW(NULL, field->data + 0x200, buff,
                                         lengthof(buff)))
                    {
                        _set_text(ctl, field->data, false);
                    }
                    else
                    {
                        SetWindowTextW(ctl, buff);
                        LoadStringW(NULL, field->data + 0x300, buff,
                                    lengthof(buff));
                        SendMessageW(ctl, BCM_SETNOTE, 0, (LPARAM)buff);
                    }
                }

                if (SendMessageW(ctl, BCM_GETIDEALSIZE, 0, (LPARAM)&ideal_size))
                {
                    SetWindowPos(ctl, NULL, 0, 0, rect.right - rect.left,
                                 ideal_size.cy, SWP_NOMOVE | SWP_NOZORDER);
                }
                cy += ideal_size.cy;
            }
        }

        if (ENCUIFT_BITMAP == field->type)
        {
            gfx_bitmap *bm = (gfx_bitmap *)field->data;
            DWORD       style = WS_VISIBLE | WS_CHILD | SS_BITMAP;
            HWND        ctl;
            int         left = 0, width, height;

            _get_scaled_dimensions(bm, &width, &height);
            if (ENCUIFF_CENTER == (ENCUIFF_ALIGN & field->flags))
            {
                left = (rect.right - rect.left - width) / 2;
            }
            else if (ENCUIFF_RIGHT == (ENCUIFF_ALIGN & field->flags))
            {
                left = rect.right - rect.left - width;
            }

            ctl = CreateWindowW(L"STATIC", L"", style, cx + left, cy, width,
                                height, dlg, (HMENU)(UINT_PTR)CPX_CTLID(i),
                                GetModuleHandleW(NULL), NULL);
            _set_scaled_bitmap(ctl, bm, width, height);
            cy += height + my;
        }
    }

    DestroyWindow(GetDlgItem(dlg, IDC_TEXT));
    if (!has_checkbox)
    {
        DestroyWindow(GetDlgItem(dlg, IDC_CHECK));
    }

    if (!has_textbox)
    {
        DestroyWindow(GetDlgItem(dlg, IDC_EDITBOX));
        DestroyWindow(GetDlgItem(dlg, IDC_BANG));
        DestroyWindow(GetDlgItem(dlg, IDC_ALERT));
    }
}

static void
_set_buttons(HWND dlg, int id, bool has_next)
{
    PropSheet_SetWizButtons(GetParent(dlg), ((0 != id) ? PSWIZB_BACK : 0) |
                                                (has_next ? PSWIZB_NEXT : 0));
}

bool
encui_refresh_field(encui_page *page, int id)
{
    encui_field *field = NULL;

    if ((_pages + _id) != page)
    {
        return false;
    }

    if ((0 > id) || (page->length <= id))
    {
        return false;
    }

    field = page->fields + id;
    if (ENCUIFT_LABEL == field->type)
    {
        HWND dlg = PropSheet_GetCurrentPageHwnd(_wnd);
        HWND ctl = GetDlgItem(dlg, CPX_CTLID(id));
        _set_text(ctl, field->data, false);
        return true;
    }

    return false;
}

bool
encui_request_notify(int cookie)
{
    return PostMessageW(_active_dlg, WM_ENCUI_NOTIFY, (WPARAM)_id,
                        (LPARAM)cookie);
}

static void
_update_controls(HWND dlg, encui_page *page)
{
    int i;
    for (i = 0; i < page->length; i++)
    {
        encui_field *field = &page->fields[i];

        if ((ENCUIFT_LABEL == field->type) && (ENCUIFF_DYNAMIC & field->flags))
        {
            HWND ctl = GetDlgItem(dlg, CPX_CTLID(i));
            _set_text(ctl, field->data, false);
        }

        if ((ENCUIFT_OPTION == field->type) && windows_is_at_least_vista())
        {
            _set_buttons(dlg, page - _pages, false);
        }

        if ((ENCUIFT_BITMAP == field->type) && (ENCUIFF_DYNAMIC & field->flags))
        {
            HWND ctl = GetDlgItem(dlg, CPX_CTLID(i));
            int  width, height;

            _get_scaled_dimensions((gfx_bitmap *)field->data, &width, &height);
            _set_scaled_bitmap(ctl, (gfx_bitmap *)field->data, width, height);
        }
    }
}

static INT_PTR CALLBACK
_dialog_proc(HWND dlg, UINT message, WPARAM wparam, LPARAM lparam)
{
    switch (message)
    {
    case WM_INITDIALOG: {
        PROPSHEETPAGEW *template = (PROPSHEETPAGEW *)lparam;
        encui_page *page = _pages + template->lParam;

        _wnd = GetParent(dlg);

        if (!windows_is_at_least_vista())
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

            // Update dialog font
            if (windows_is_less_than_2000() && (NULL == _font))
            {
                NONCLIENTMETRICSA nc_metrics = {sizeof(NONCLIENTMETRICSA)};
                if (SystemParametersInfoA(SPI_GETNONCLIENTMETRICS,
                                          sizeof(nc_metrics), &nc_metrics, 0))
                {
                    _prev_font = (HFONT)SendMessageA(dlg, WM_GETFONT, 0, 0);
                    _font = CreateFontIndirectA(&nc_metrics.lfMessageFont);
                    if (NULL != _font)
                    {
                        HDC dc;

                        _allocated_font = true;
                        SendMessageW(dlg, WM_SETFONT, (WPARAM)_font, FALSE);

                        if (NULL != (dc = GetDC(dlg)))
                        {
                            TEXTMETRICA txt_metrics = {0};

                            SelectObject(dc, _prev_font);
                            GetTextMetricsA(dc, &txt_metrics);
                            _prev_avg_width = txt_metrics.tmAveCharWidth;

                            SelectObject(dc, _font);
                            GetTextMetricsA(dc, &txt_metrics);
                            _avg_width = txt_metrics.tmAveCharWidth;

                            ReleaseDC(dlg, dc);
                        }
                    }
                }
            }

            // Apply padding
            MapDialogRect(dlg, &padding);
            padding.left = padding.left * _avg_width / _prev_avg_width;
            padding.top = padding.top * _avg_width / _prev_avg_width;
            padding.right = padding.right * _avg_width / _prev_avg_width;
            padding.bottom = padding.bottom * _avg_width / _prev_avg_width;

            ctl = GetWindow(dlg, GW_CHILD);
            while (ctl)
            {
                RECT ctl_rect;
                GetWindowRect(ctl, &ctl_rect);
                MapWindowPoints(NULL, dlg, (LPPOINT)&ctl_rect, 2);

                SetWindowPos(ctl, NULL, ctl_rect.left + padding.left,
                             ctl_rect.top + padding.top,
                             (ctl_rect.right - ctl_rect.left) * _avg_width /
                                 _prev_avg_width,
                             ctl_rect.bottom - ctl_rect.top, SWP_NOZORDER);

                ctl = GetWindow(ctl, GW_HWNDNEXT);
            }
        }

        if (NULL == _font)
        {
            _font = (HFONT)SendMessageA(dlg, WM_GETFONT, 0, 0);
        }

        _active_dlg = dlg;
        _set_text(dlg, page->title, false);
        _create_controls(dlg, page);
        encui_check_page(page, NULL);

        return TRUE;
    }

    case WM_TIMER: {
        if (IDT_ENTERED == wparam)
        {
            KillTimer(dlg, IDT_ENTERED);
            _pages[_id].proc(ENCUIM_ENTERED, NULL, _pages[_id].data);
        }

        return 0;
    }

    case WM_ENCUI_NOTIFY: {
        int id = (int)wparam;
        if (id != PropSheet_HwndToIndex(GetParent(dlg), dlg))
        {
            return 0;
        }

        _pages[id].proc(ENCUIM_NOTIFY, (void *)lparam, _pages[id].data);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        POINT point = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        HWND  ctl = ChildWindowFromPoint(dlg, point);

        int ctl_id = GetDlgCtrlID(ctl);
        if (0x100 <= ctl_id)
        {
            WCHAR ctl_cn[256];
            GetClassNameW(ctl, ctl_cn, lengthof(ctl_cn));
            if (0 == wcsicmp(ctl_cn, L"STATIC"))
            {
                _pages[_id].proc(ENCUIM_NOTIFY, (void *)(intptr_t)ctl_id,
                                 _pages[_id].data);
            }
        }

        return 0;
    }

    case WM_NOTIFY: {
        int     id = PropSheet_HwndToIndex(GetParent(dlg), dlg);
        LPNMHDR notif = (LPNMHDR)lparam;
        switch (notif->code)
        {
        case PSN_SETACTIVE: {
            _active_dlg = dlg;
            _pages[id].proc(ENCUIM_INIT, NULL, _pages[id].data);
            _check_input(dlg, id);
            _update_controls(dlg, _pages + id);
            SetTimer(dlg, IDT_ENTERED, USER_TIMER_MINIMUM, NULL);
            return 0;
        }

        case PSN_WIZBACK: {
            if (0 != _pages[id - 1].title)
            {
                _id = id - 1;
                break;
            }

            PropSheet_SetCurSel(GetParent(dlg), NULL,
                                (intptr_t)_pages[id - 1].data);
            SetWindowLongPtrW(dlg, DWLP_MSGRESULT, -1);
            return TRUE;
        }

        case PSN_WIZNEXT: {
            int                 status;
            encui_textbox_data *textbox;
            HWND                edit_box = GetDlgItem(dlg, IDC_EDITBOX);
            size_t              length = GetWindowTextLengthW(edit_box);
            LPWSTR              text;

            textbox = encui_find_textbox(_pages + id);
            if (NULL != textbox)
            {
                text = (LPWSTR)malloc((length + 1) * sizeof(WCHAR));
                if (NULL == text)
                {
                    return -1;
                }
                GetWindowTextW(edit_box, text, length + 1);
                WideCharToMultiByte(CP_UTF8, 0, text, -1, textbox->buffer,
                                    textbox->capacity, NULL, NULL);
                _value = length;
                free(text);
            }

            status = _pages[id].proc(
                ENCUIM_NEXT, textbox ? textbox->buffer : NULL, _pages[id].data);
            if (0 < status)
            {
                WCHAR message[MAX_PATH];
                HWND  alert = GetDlgItem(dlg, IDC_ALERT);

                if (INT_MAX == status)
                {
                    MultiByteToWideChar(CP_UTF8, 0, textbox->alert, -1, message,
                                        lengthof(message));
                    free((void *)textbox->alert);
                    textbox->alert = NULL;
                }
                else
                {
                    LoadStringW(NULL, status, message, lengthof(message));
                }

                if (NULL == alert)
                {
                    int label_id;
                    for (label_id = 0; label_id < _pages[id].length; label_id++)
                    {
                        if (ENCUIFT_LABEL == _pages[id].fields[label_id].type)
                        {
                            alert = GetDlgItem(dlg, CPX_CTLID(label_id));
                            break;
                        }
                    }
                }
                SetWindowTextW(alert, message);
                Static_SetIcon(GetDlgItem(dlg, IDC_BANG), _bang);
                MessageBeep(MB_ICONEXCLAMATION);
                _set_buttons(dlg, id, false);

                SetFocus(edit_box);
                SetWindowLongPtrW(dlg, DWLP_MSGRESULT, -1);
                return TRUE;
            }

            if ((0 > status) && (-ENOSYS != status))
            {
                SetWindowLongPtrW(dlg, DWLP_MSGRESULT, -1);
                return TRUE;
            }

            SetWindowTextW(GetDlgItem(dlg, IDC_ALERT), L"");
            Static_SetIcon(GetDlgItem(dlg, IDC_BANG), NULL);

            if (0 != _pages[id + 1].title)
            {
                _id = id + 1;
                return TRUE;
            }

            PropSheet_PressButton(GetParent(dlg), PSBTN_FINISH);
            SetWindowLongPtrW(dlg, DWLP_MSGRESULT, -1);
            return TRUE;
        }

        case PSN_QUERYCANCEL: {
            _value = 0;
            return FALSE;
        }

        case NM_CLICK: {
            int ctl_id = GetDlgCtrlID(notif->hwndFrom);
            if (0x100 <= ctl_id)
            {
                _pages[id].proc(ENCUIM_NOTIFY, (void *)(intptr_t)ctl_id,
                                _pages[id].data);
            }
            break;
        }
        }

        break;
    }

    case WM_COMMAND: {
        int id = PropSheet_HwndToIndex(GetParent(dlg), dlg);

        if ((EN_CHANGE == HIWORD(wparam)) && (IDC_EDITBOX == LOWORD(wparam)))
        {
            _set_buttons(dlg, id, _check_input(dlg, id));
            return TRUE;
        }

        if ((BN_CLICKED == HIWORD(wparam)) && (IDC_CHECK == LOWORD(wparam)))
        {
            encui_field *checkbox = encui_find_checkbox(_pages + id);
            int          state = Button_GetCheck(GetDlgItem(dlg, IDC_CHECK));
            if (BST_CHECKED == state)
            {
                checkbox->flags |= ENCUIFF_CHECKED;
            }
            else
            {
                checkbox->flags &= ~ENCUIFF_CHECKED;
            }
            return TRUE;
        }

        if ((BN_CLICKED == HIWORD(wparam)) && (0x100 < LOWORD(wparam)))
        {
            int i, ctl_idx = LOWORD(wparam) - 0x100;

            if (_pages[id].length <= ctl_idx)
            {
                return FALSE;
            }

            if (ENCUIFT_OPTION != _pages[id].fields[ctl_idx].type)
            {
                return FALSE;
            }

            for (i = 0; i < _pages[id].length; i++)
            {
                if (ENCUIFT_OPTION == _pages[id].fields[i].type)
                {
                    _pages[id].fields[i].flags &= ~ENCUIFF_CHECKED;
                }
            }

            _pages[id].fields[ctl_idx].flags |= ENCUIFF_CHECKED;
            if (windows_is_at_least_vista())
            {
                PropSheet_PressButton(GetParent(dlg), PSBTN_NEXT);
            }

            return TRUE;
        }
    }
    }

    return FALSE;
}

static HICON
_find_bang(void)
{
    if (NULL != _bang)
    {
        return _bang;
    }

    // Vista or newer - icon from the Shell API
    if (windows_is_at_least_vista())
    {
        SHSTOCKICONINFO       info = {sizeof(SHSTOCKICONINFO)};
        pf_shgetstockiconinfo fn = NULL;

#if WINVER >= 0x0600
        fn = SHGetStockIconInfo;
#else
        fn = windows_get_proc("shell32.dll", "SHGetStockIconInfo",
                              pf_shgetstockiconinfo);
#endif
        if (fn &&
            SUCCEEDED(fn(SIID_WARNING, SHGSI_ICON | SHGSI_SMALLICON, &info)))
        {
            if (NULL != info.hIcon)
            {
                return info.hIcon;
            }
        }
    }

    // XP and 2003 (Common Controls 6.0) - shaded
    // 98 SE, Me and 2000 (Common Controls 5.x) - flat
    return (HICON)LoadImageA(GetModuleHandleA("comctl32.dll"),
                             MAKEINTRESOURCEA(20482), IMAGE_ICON,
                             GetSystemMetrics(SM_CXSMICON),
                             GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
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
            _psps[i].dwSize = sizeof(PROPSHEETPAGEW);
            _psps[i].hInstance = GetModuleHandleW(NULL);
            _psps[i].dwFlags = PSP_USEHEADERTITLE | PSP_USETITLE;
            _psps[i].lParam = (LPARAM)i;
            _psps[i].pszHeaderTitle = L"Vacat";
            _psps[i].pszTemplate = MAKEINTRESOURCEW(IDD_VACAT);
            _psps[i].pszTitle = _brand;
            _psps[i].pfnDlgProc = NULL;
            _hpsps[i] = CreatePropertySheetPageW((LPCPROPSHEETPAGEW)&_psps[i]);
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
    _psh.dwFlags = windows_is_at_least_vista()
                       ? PSH_WIZARD | PSH_AEROWIZARD | PSH_USEICONID
                       : PSH_WIZARD97 | PSH_HEADER;
    _psh.pszCaption = _brand;
    _psh.pszIcon = MAKEINTRESOURCEW(1);
    _psh.pszbmHeader = MAKEINTRESOURCEW(IDB_HEADER);
    _psh.nStartPage = 0;
    _psh.nPages = count;

    _pages = pages;
    _id = -1;

    _bang = _find_bang();
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

    if (_allocated_font)
    {
        DeleteObject(_font);
        _font = NULL;
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

    if (0 != _value)
    {
        _id = id;
        PropSheet_SetCurSel(_wnd, NULL, id);
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
    _wnd = NULL;
    return value;
}

int
encui_check_page(const encui_page *page, void *param)
{
    int status = page->proc(ENCUIM_CHECK, param, page->data);
    _set_buttons(_active_dlg, _id, 0 >= status);
    return status;
}
