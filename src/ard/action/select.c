#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <ard/action.h>
#include <ard/ui.h>
#include <ard/version.h>

#include "../resource.h"

#define WINDOW_WIDTH  480
#define WINDOW_HEIGHT 360
#define WINDOW_MARGIN 10
#define WINDOW_MAXW   (WINDOW_WIDTH - 2 * WINDOW_MARGIN)
#define WS_LARD       (WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX)

#define BUTTON_LEFT (110 + WINDOW_MARGIN)
#define BUTTON_SIZE 48

#define ID_INSTREDIST 101
#define ID_RUNDOS     102
#define ID_INSTIE     103
#define ID_INTRO      201
#define ID_TITLE      202
#define ID_TEXT       203
#define ID_FOOTER     204

typedef BOOL(WINAPI *pf_systemparametersinfo)(UINT, UINT, PVOID, UINT);
typedef HANDLE(WINAPI *pf_loadimage)(HINSTANCE, LPCSTR, UINT, int, int, UINT);

// Window class names
static const char WC_LARD[] = "LARD Window Class";

static const char WC_BUTTON[] = "Button";
static const char WC_STATIC[] = "Static";

// External file paths
static const char FP_BG[] = "bg.bmp";
static const char FP_BG16[] = "bg16.bmp";
static const char FP_ICON[] = "icon.ico";

// Model
static const ardc_config *config_;
static char              *listing_;
static int                listing_length_;
static ardc_source      **sources_;

// Window state
static HBITMAP          bg_;
static HFONT            font_;
static HFONT            font_bold_;
static HICON            icon_ = NULL;
static HICON            smicon_ = NULL;
static HINSTANCE        instance_;
static NONCLIENTMETRICS nc_metrics_ = {sizeof(NONCLIENTMETRICS)};
static HWND             wnd_;
static RECT             wnd_rect_ = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

static HANDLE
load_image(_In_opt_ HINSTANCE instance,
           _In_ LPCSTR        name,
           _In_ UINT          type,
           _In_ int           cx,
           _In_ int           cy,
           _In_ UINT          flags)
{
    HMODULE      user32;
    pf_loadimage fn_li;

    user32 = GetModuleHandle("user32.dll");
    fn_li =
        (pf_loadimage)(user32 ? GetProcAddress(user32, "LoadImageA") : NULL);

    return fn_li ? fn_li(instance, name, type, cx, cy, flags) : NULL;
}

static HBITMAP
load_bitmap_from_file(_In_z_ const char *path)
{
    return (HBITMAP)load_image(NULL, path, IMAGE_BITMAP, 0, 0,
                               LR_CREATEDIBSECTION | LR_DEFAULTSIZE |
                                   LR_LOADFROMFILE);
}

static HICON
load_icon_from_file(_In_z_ const char *path, _In_ int cx, _In_ int cy)
{
    return (HICON)load_image(NULL, path, IMAGE_ICON, cx, cy,
                             ((cx || cy) ? 0 : LR_DEFAULTSIZE) |
                                 LR_LOADFROMFILE);
}

static HICON
load_icon_from_resource(_In_ int id, _In_ int cx, _In_ int cy)
{
    HICON icon = (HICON)load_image(
        GetModuleHandle(NULL), MAKEINTRESOURCE(id), IMAGE_ICON, cx, cy,
        ((cx || cy) ? 0 : LR_DEFAULTSIZE) | LR_SHARED);
    return icon ? icon : LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(id));
}

static void
load_fonts(void)
{
    HMODULE                 user32;
    pf_systemparametersinfo fn_spi;

    user32 = GetModuleHandle("user32.dll");
    fn_spi =
        (pf_systemparametersinfo)(user32 ? GetProcAddress(
                                               user32, "SystemParametersInfoA")
                                         : NULL);
    if (fn_spi &&
        fn_spi(SPI_GETNONCLIENTMETRICS, sizeof(nc_metrics_), &nc_metrics_, 0))
    {
        font_ = CreateFontIndirect(&nc_metrics_.lfMessageFont);

        nc_metrics_.lfMessageFont.lfWeight = FW_BOLD;
        font_bold_ = CreateFontIndirect(&nc_metrics_.lfMessageFont);
        nc_metrics_.lfMessageFont.lfWeight = FW_NORMAL;
    }
}

static int
count_sources(_In_ ardc_source **sources)
{
    ardc_source **ptr = sources;

    while (*ptr)
    {
        ptr++;
    }

    return ptr - sources;
}

static bool
measure_text(_In_ HWND        ctl,
             _In_ const char *text,
             _In_ HFONT       font,
             _Out_ RECT      *rect)
{
    HDC dc = GetDC(ctl);

    SelectObject(dc, font);
    GetClientRect(ctl, rect);
    DrawText(dc, text, -1, rect, DT_CALCRECT | DT_WORDBREAK);
    ReleaseDC(ctl, dc);
    return true;
}

static void
resize_control(_In_ HWND ctl, _In_ int cx, _In_ int cy)
{
    SetWindowPos(ctl, NULL, 0, 0, cx, cy,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

static int
create_option(_In_ unsigned      id,
              _In_opt_ HICON     icon,
              _In_z_ const char *title,
              _In_z_ const char *description,
              _In_ int           cy,
              _In_ HWND          parent)
{
    HWND ctl;
    RECT rect;
    int  cx = WINDOW_MARGIN + BUTTON_LEFT;

    ctl = CreateWindow(
        WC_BUTTON, "",
        WS_CHILD | WS_TABSTOP | WS_VISIBLE | BS_PUSHBUTTON | BS_ICON, cx, cy,
        BUTTON_SIZE, BUTTON_SIZE, parent, (HMENU)id, instance_, 0);
    SendMessage(ctl, BM_SETIMAGE, IMAGE_ICON, (LPARAM)icon);
    cx += BUTTON_SIZE + WINDOW_MARGIN;

    ctl = CreateWindow(WC_STATIC, title, WS_VISIBLE | WS_CHILD | SS_LEFT, cx,
                       cy, WINDOW_MAXW - cx - WINDOW_MARGIN, BUTTON_SIZE,
                       parent, (HMENU)ID_TITLE, instance_, 0);
    SendMessage(ctl, WM_SETFONT, (WPARAM)font_bold_, TRUE);
    measure_text(ctl, title, font_bold_, &rect);
    resize_control(ctl, rect.right, rect.bottom);
    cy += rect.bottom;

    ctl = CreateWindow(WC_STATIC, description, WS_VISIBLE | WS_CHILD | SS_LEFT,
                       cx, cy, WINDOW_MAXW - cx - WINDOW_MARGIN, BUTTON_SIZE,
                       parent, (HMENU)ID_TEXT, instance_, 0);
    SendMessage(ctl, WM_SETFONT, (WPARAM)font_, TRUE);
    measure_text(ctl, description, font_, &rect);
    resize_control(ctl, rect.right, rect.bottom);
    cy += rect.bottom;

    return cy;
}

static int
create_option_redists(_In_ int cy, _In_ HWND parent, _In_ ardc_source **sources)
{
    char          message[ARDC_LENGTH_MID] = "";
    char          format[ARDC_LENGTH_MID] = "";
    ardc_source **src;
    HICON         icon;

    LoadString(NULL, config_->ie_offer ? IDS_INSTREDISTO : IDS_INSTREDIST,
               message, ARRAYSIZE(message));

    if (1 == count_sources(sources_))
    {
        LoadString(NULL, IDS_INSTREDISTD, format, ARRAYSIZE(format));
        sprintf(listing_, format, sources_[0]->description);
    }
    else
    {
        LoadString(NULL, IDS_INSTREDISTS, listing_, listing_length_);
        for (src = sources_; *src; src++)
        {
            if (sources_ != src)
            {
                strcat(listing_, ", ");
            }

            strcat(listing_, (*src)->description);
        }
    }

    icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_INSTREDIST));
    cy = create_option(ID_INSTREDIST, icon, message, listing_, cy, parent);

    LocalFree(listing_);
    return cy;
}

static int
create_option_rundos(_In_ int cy, _In_ HWND parent)
{
    char  title[ARDC_LENGTH_MID] = "";
    char  description[ARDC_LENGTH_LONG] = "";
    HICON icon;

    LoadString(NULL, IDS_RUNDOS, title, ARRAYSIZE(title));
    LoadString(NULL, IDS_RUNDOSD, description, ARRAYSIZE(description));

    icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_RUNDOS));
    return create_option(ID_RUNDOS, icon, title, description, cy, parent);
}

static int
create_option_ie(_In_ int cy, _In_ HWND parent)
{
    char  title[ARDC_LENGTH_MID] = "";
    char  description[ARDC_LENGTH_LONG] = "";
    char  format[ARDC_LENGTH_LONG] = "";
    HICON icon;
    bool  has_ie = ardv_ie_available();

    LoadString(NULL, has_ie ? IDS_INSTIEUP : IDS_INSTIE, title,
               ARRAYSIZE(title));
    LoadString(NULL, has_ie ? IDS_INSTIEUPD : IDS_INSTIED, format,
               ARRAYSIZE(format));
    sprintf(description, format, config_->ie_description);

    icon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_INSTIE));
    return create_option(ID_INSTIE, icon, title, description, cy, parent);
}

static LRESULT
wndproc_create(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    HWND  ctl;
    int   cy = WINDOW_MARGIN, cy_new;
    char  format[ARDC_LENGTH_LONG] = "";
    HICON icon;
    char  message[ARDC_LENGTH_LONG] = "";
    RECT  rect;
    int   cxsmicon = GetSystemMetrics(SM_CXSMICON),
        cysmicon = GetSystemMetrics(SM_CYSMICON),
        cxicon = GetSystemMetrics(SM_CXICON),
        cyicon = GetSystemMetrics(SM_CYICON);

    {
        HDC dc = GetDC(wnd);
        int bpp = GetDeviceCaps(dc, BITSPIXEL) * GetDeviceCaps(dc, PLANES);
        ReleaseDC(wnd, dc);

        if (8 > bpp)
        {
            bg_ = load_bitmap_from_file(FP_BG16);
        }
        bg_ = bg_ ? bg_ : load_bitmap_from_file(FP_BG);
        bg_ = bg_ ? bg_
                  : LoadBitmap(GetModuleHandle(NULL),
                               (8 > bpp) ? MAKEINTRESOURCE(IDB_BG16)
                                         : MAKEINTRESOURCE(IDB_BG));
    }

    icon_ = load_icon_from_file(FP_ICON, cxicon, cyicon);
    icon = icon_ ? icon_ : load_icon_from_resource(IDI_APPICON, cxicon, cyicon);
    SendMessageW(wnd, WM_SETICON, ICON_BIG, (LPARAM)icon);

    smicon_ = load_icon_from_file(FP_ICON, cxsmicon, cysmicon);
    icon = smicon_ ? smicon_
                   : load_icon_from_resource(IDI_APPICON, cxsmicon, cysmicon);
    SendMessageW(wnd, WM_SETICON, ICON_SMALL, (LPARAM)icon);

    LoadString(NULL, IDS_INFORMATION, format, ARRAYSIZE(format));
    sprintf(message, format, config_->name);
    ctl =
        CreateWindow(WC_STATIC, message, WS_VISIBLE | WS_CHILD | SS_LEFT,
                     WINDOW_MARGIN + BUTTON_LEFT, cy, WINDOW_MAXW - BUTTON_LEFT,
                     BUTTON_SIZE, wnd, (HMENU)ID_INTRO, instance_, 0);
    SendMessage(ctl, WM_SETFONT, (WPARAM)font_, TRUE);
    measure_text(ctl, message, font_, &rect);
    resize_control(ctl, rect.right, rect.bottom);
    cy += rect.bottom + 2 * WINDOW_MARGIN;

    if (config_->ie_offer > ardv_ie_get_version())
    {
        cy_new = create_option_ie(cy, wnd);
        cy = max(cy_new, cy + BUTTON_SIZE) + 2 * WINDOW_MARGIN;
    }

    cy_new = create_option_redists(cy, wnd, sources_);
    cy = max(cy_new, cy + BUTTON_SIZE) + 2 * WINDOW_MARGIN;

    if (arda_rundos_available(config_))
    {
        cy_new = create_option_rundos(cy, wnd);
        cy = max(cy_new, cy + BUTTON_SIZE) + 2 * WINDOW_MARGIN;
    }

    ctl = CreateWindow(WC_STATIC, config_->copyright,
                       WS_VISIBLE | WS_CHILD | SS_LEFT, WINDOW_MARGIN,
                       WINDOW_HEIGHT - WINDOW_MARGIN, WINDOW_MAXW, BUTTON_SIZE,
                       wnd, (HMENU)ID_FOOTER, instance_, 0);
    SendMessage(ctl, WM_SETFONT, (WPARAM)font_, TRUE);
    measure_text(ctl, config_->copyright, font_, &rect);
    MoveWindow(ctl, WINDOW_MARGIN, WINDOW_HEIGHT - WINDOW_MARGIN - rect.bottom,
               rect.right, rect.bottom, TRUE);
    SetFocus(wnd);

    return DefWindowProc(wnd, msg, wparam, lparam);
}

static LRESULT CALLBACK
wndproc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_CREATE:
        return wndproc_create(wnd, msg, wparam, lparam);

    case WM_DESTROY: {
        PostQuitMessage(0);
        break;
    }

    case WM_GETMINMAXINFO: {
        // prevent resizing
        MINMAXINFO *mmi = (MINMAXINFO *)lparam;
        mmi->ptMinTrackSize.x = mmi->ptMaxTrackSize.x =
            wnd_rect_.right - wnd_rect_.left;
        mmi->ptMinTrackSize.y = mmi->ptMaxTrackSize.y =
            wnd_rect_.bottom - wnd_rect_.top;
        return 0;
    }

    case WM_COMMAND: {
        if (BN_CLICKED == HIWORD(wparam))
        {
            int id = LOWORD(wparam);

            if (ID_INSTREDIST == id)
            {
                if (0 == arda_instredist(config_, sources_))
                {
                    arda_run(config_);
                }

                break;
            }

            if (ID_RUNDOS == id)
            {
                arda_rundos(config_);
                break;
            }

            if (ID_INSTIE == id)
            {
                arda_instie(config_);
                break;
            }
        }

        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC         dc = BeginPaint(wnd, &ps);

        HDC bmp_dc = CreateCompatibleDC(dc);
        SelectObject(bmp_dc, bg_);

        BITMAP bmp;
        GetObject(bg_, sizeof(BITMAP), &bmp);
        BitBlt(dc, 0, 0, bmp.bmWidth, bmp.bmHeight, bmp_dc, 0, 0, SRCCOPY);

        DeleteDC(bmp_dc);
        EndPaint(wnd, &ps);
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC dc = (HDC)wparam;
        int id = GetDlgCtrlID((HWND)lparam);

        SetBkMode(dc, TRANSPARENT);

        COLORREF color = (ID_TITLE == id)    ? config_->title_color
                         : (ID_FOOTER == id) ? config_->footer_color
                         : (ID_INTRO == id)  ? config_->intro_color
                                             : config_->text_color;
        SetTextColor(dc, color);

        return (LRESULT)GetStockObject(NULL_BRUSH);
    }
    }

    return DefWindowProc(wnd, msg, wparam, lparam);
}

static HWND
create_window(HINSTANCE instance)
{
    HWND     wnd = NULL;
    WNDCLASS wndc = {0};

    wndc.lpfnWndProc = wndproc;
    wndc.hInstance = instance;
    wndc.lpszClassName = WC_LARD;
    if (0 == RegisterClass(&wndc))
    {
        // cannot register the window class
        return NULL;
    }

    load_fonts();

    AdjustWindowRect(&wnd_rect_, WS_LARD, FALSE);
    wnd = CreateWindow(WC_LARD,                          // window class
                       ardui_get_title(),                // window text
                       WS_LARD,                          // window style
                       CW_USEDEFAULT, CW_USEDEFAULT,     // position
                       wnd_rect_.right - wnd_rect_.left, // width
                       wnd_rect_.bottom - wnd_rect_.top, // height
                       NULL,                             // parent
                       NULL,                             // menu
                       instance,                         // application instance
                       NULL);
    if (NULL == wnd)
    {
        // cannot create the window
        UnregisterClass(WC_LARD, instance);
    }

    return wnd;
}

static void
destroy_window(HINSTANCE instance, HWND wnd)
{
    DestroyWindow(wnd);
    UnregisterClass(WC_LARD, instance);

    DeleteObject(font_);
    DeleteObject(font_bold_);
    font_ = font_bold_ = NULL;
}

int
arda_select(_In_ const ardc_config *cfg, _In_ ardc_source **sources)
{
    char   message[ARDC_LENGTH_LONG] = "";
    char  *listing;
    size_t listing_length;

    listing_length =
        (ARDC_LENGTH_MID + ARDC_LENGTH_LONG) * (cfg->srcs_count + 2);
    listing = (char *)LocalAlloc(LMEM_FIXED, listing_length);
    if (NULL == listing)
    {
        LoadString(NULL, IDS_REDIST, message, ARRAYSIZE(message));
        strcat(message, ".");

        MessageBox(NULL, message, ardui_get_title(), MB_ICONERROR | MB_OK);
        PostQuitMessage(0);
        return 0;
    }

    config_ = cfg;
    listing_ = listing;
    listing_length_ = listing_length;
    sources_ = sources;
    instance_ = GetModuleHandle(NULL);
    wnd_ = create_window(instance_);

    ShowWindow(wnd_, SW_SHOWNORMAL);

    return 0;
}

void
arda_select_cleanup(void)
{
    if (wnd_)
    {
        destroy_window(GetModuleHandle(NULL), wnd_);
    }

    if (sources_)
    {
        LocalFree(sources_);
    }

    if (icon_)
    {
        DestroyIcon(icon_);
    }

    if (smicon_)
    {
        DestroyIcon(smicon_);
    }
}

HWND
arda_select_get_window(void)
{
    return wnd_;
}
