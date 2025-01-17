#include <math.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define UNICODE
#include <shlobj.h>
#include <windows.h>
#include <windowsx.h>
#include <wininet.h>

#include <fmt/utf8.h>
#include <pal.h>
#include <platform/windows.h>
#include <snd.h>

#include "../resource.h"
#include "evtmouse.h"
#include "pal_impl.h"

#if defined(_MSC_VER) ||                                                       \
    (defined(__USE_MINGW_ANSI_STDIO) && !__USE_MINGW_ANSI_STDIO)
#define FMT_AS L"%S"
#else
#define FMT_AS L"%s"
#endif

#ifndef INFINITY
#define INFINITY 1000.f
#endif

#define ID_ABOUT 0x1000
#define ID_SCALE 0x2000
#define ID_FULL  0x2100

// See DEVICE_SCALE_FACTOR in shtypes.h
static const float SCALES[] = {1.00f, 1.20f, 1.25f, 1.40f, 1.50f, 1.60f,
                               1.75f, 1.80f, 2.00f, 2.25f, 2.50f, 3.00f,
                               3.50f, 4.00f, 4.50f, 5.00f};

static const wchar_t *FONT_NAMES[] = {
    L"Cascadia Code",  // Windows 11
    L"Consolas",       // Windows Vista
    L"Lucida Console", // Windows 2000
    NULL               // usually Courier New
};

static LPVOID _ver_resource = NULL;
static WORD  *_ver_vfi_translation = NULL;
static char   _ver_string[MAX_PATH] = {0};

static HICON _icon = NULL;
static HWND  _wnd = NULL;
static char *_font = NULL;
static DWORD _start_time = 0;

static HINSTANCE _instance = NULL;
static int       _cmd_show;
static bool      _no_stall = false;
static uint16_t  _version;

static HINTERNET     _session = NULL;
static HINTERNET     _connection = NULL;
static HINTERNET     _request = NULL;
static palinet_proc *_inet_proc = NULL;
static DWORD_PTR     _inet_data = 0;

static HHOOK   _hook = NULL;
static WNDPROC _prev_wnd_proc = NULL;
static HMENU   _sys_menu = NULL;
static HMENU   _size_menu = NULL;

static WPARAM _keycode;

static gfx_dimensions _mouse_cell;

static int _min_scale_id = 0;

static bool            _fullscreen = false;
static WINDOWPLACEMENT _placement = {sizeof(WINDOWPLACEMENT)};
static float           _window_scale = 0.f;

extern int
__mme_init(void);

extern int
main(int argc, char *argv[]);

int WINAPI
wWinMain(_In_ HINSTANCE     instance,
         _In_opt_ HINSTANCE prev_instance,
         _In_ PWSTR         cmd_line,
         _In_ int           cmd_show)
{
    int    argc = __argc, i, status;
    char **argv = (char **)malloc(((size_t)argc + 2) * sizeof(char *));
    if (NULL == argv)
    {
        return errno;
    }

    _instance = instance;
    _cmd_show = cmd_show;

    for (i = 0; i < argc; i++)
    {
        size_t length = WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, NULL, 0,
                                            NULL, NULL);
        argv[i] = (char *)malloc(length + 1);
        if (NULL == argv[i])
        {
            argc = i;
            break;
        }
        WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, argv[i], length + 1,
                            NULL, NULL);
    }
    argv[argc] = NULL;

    status = main(argc, argv);

    for (i = 0; i < argc; i++)
    {
        free(argv[i]);
    }
    free(argv);
    return status;
}

static BOOL CALLBACK
_about_enum_child_proc(HWND wnd, LPARAM lparam)
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
_about_hook_wnd_proc(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT rc = CallWindowProc(_prev_wnd_proc, wnd, message, wparam, lparam);

    if (message == WM_INITDIALOG)
    {
        HWND icon_wnd = NULL;
        RECT parent_rect, msgbox_rect;
        LONG parent_width, parent_height, msgbox_width, msgbox_height;

        GetWindowRect(windows_get_hwnd(), &parent_rect);
        parent_width = parent_rect.right - parent_rect.left;
        parent_height = parent_rect.bottom - parent_rect.top;

        GetWindowRect(wnd, &msgbox_rect);
        msgbox_width = msgbox_rect.right - msgbox_rect.left;
        msgbox_height = msgbox_rect.bottom - msgbox_rect.top;

        MoveWindow(wnd, parent_rect.left + (parent_width - msgbox_width) / 2,
                   parent_rect.top + (parent_height - msgbox_height) / 2,
                   msgbox_width, msgbox_height, FALSE);

        EnumChildWindows(wnd, _about_enum_child_proc, (LPARAM)&icon_wnd);
        if (NULL != icon_wnd)
        {
            HICON icon = LoadIconW(_instance, MAKEINTRESOURCEW(2));
            Static_SetIcon(icon_wnd, icon);
        }

        return rc;
    }

    if (message == WM_NCDESTROY)
    {
        UnhookWindowsHookEx(_hook);
    }

    return rc;
}

static LRESULT CALLBACK
_about_hook_proc(int code, WPARAM wparam, LPARAM lparam)
{
    if (HC_ACTION == code)
    {
        LPCWPSTRUCT cwp = (LPCWPSTRUCT)lparam;
        if (cwp->message == WM_INITDIALOG)
        {
            _prev_wnd_proc = (WNDPROC)SetWindowLongPtrW(
                cwp->hwnd, GWLP_WNDPROC, (LONG_PTR)_about_hook_wnd_proc);
        }
    }

    return CallNextHookEx(_hook, code, wparam, lparam);
}

static void
_append(wchar_t *dst, const wchar_t *src, size_t size)
{
    wcsncat(dst, src, size - wcslen(dst));
}

static int
_find_scale(float scale, int direction)
{
    float best_delta = INFINITY;
    int   scale_idx = 0, i;

    if (0 == direction)
    {
        return _min_scale_id;
    }

    for (i = 0; i < lengthof(SCALES); i++)
    {
        float delta = fabsf(SCALES[i] - scale);
        if (best_delta > delta)
        {
            best_delta = delta;
            scale_idx = i;
        }
    }

    return min(max(scale_idx + direction, _min_scale_id - ID_SCALE),
               lengthof(SCALES) - 1);
}

HFONT
windows_find_font(int max_width, int max_height)
{

    HDC            dc;
    HFONT          font = NULL;
    TEXTMETRICW    metric;
    const wchar_t *family = NULL;
    int            i, min_height;

    // Determine the font family
    for (i = 0; i < lengthof(FONT_NAMES); i++)
    {
        font = CreateFontW(max_height, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                           ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN,
                           FONT_NAMES[i]);
        if (NULL != font)
        {
            family = FONT_NAMES[i];
            break;
        }
    }

    if (NULL == font)
    {
        return NULL;
    }

    dc = windows_get_dc();

    // Try the largest height
    SelectObject(dc, font);
    GetTextMetricsW(dc, &metric);
    if ((0 > max_width) || (max_width >= metric.tmAveCharWidth))
    {
        return font;
    }

    // Continue with binary search, fit width
    min_height = 0;
    while (1 < (max_height - min_height))
    {
        int height = (min_height + max_height) / 2;
        DeleteObject(font);

        font =
            CreateFontW(height, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
                        DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                        ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, family);
        SelectObject(dc, font);
        GetTextMetricsW(dc, &metric);
        if (max_width < metric.tmAveCharWidth)
        {
            max_height = height;
        }
        else
        {
            min_height = height;
        }
    }

    DeleteObject(font);
    font = CreateFontW(min_height, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
                       DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                       ANTIALIASED_QUALITY, FIXED_PITCH | FF_MODERN, family);
    return font;
}

static bool
_set_scale(int idx)
{
    idx = min(max(_min_scale_id - ID_SCALE, idx), lengthof(SCALES) - 1);
    if (!windows_set_scale(SCALES[idx]))
    {
        return false;
    }

    gfx_get_glyph_dimensions(&_mouse_cell);
    CheckMenuRadioItem(_sys_menu, _min_scale_id,
                       ID_SCALE + lengthof(SCALES) - 1, ID_SCALE + idx,
                       MF_BYCOMMAND);
    return true;
}

static void
_toggle_fullscreen(HWND wnd)
{
    int i;

    DWORD style = GetWindowLongW(wnd, GWL_STYLE);
    if (_fullscreen)
    {
        windows_set_scale(_window_scale);
        SetWindowLongW(wnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(wnd, &_placement);
        SetWindowPos(wnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    else
    {
        MONITORINFO mi = {sizeof(mi)};
        if (GetWindowPlacement(wnd, &_placement) &&
            GetMonitorInfoW(MonitorFromWindow(wnd, MONITOR_DEFAULTTOPRIMARY),
                            &mi))
        {
            int max_width =
                (mi.rcMonitor.right - mi.rcMonitor.left) / GFX_COLUMNS;
            int max_height =
                (mi.rcMonitor.bottom - mi.rcMonitor.top) / GFX_LINES;

            _window_scale = gfx_get_scale();
            windows_set_font(windows_find_font(max_width, max_height));
            windows_set_box(mi.rcMonitor.right - mi.rcMonitor.left,
                            mi.rcMonitor.bottom - mi.rcMonitor.top);
            SetWindowLongW(wnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(wnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }

    gfx_get_glyph_dimensions(&_mouse_cell);
    _fullscreen = !_fullscreen;

    CheckMenuItem(_size_menu, ID_FULL,
                  MF_BYCOMMAND | (_fullscreen ? MF_CHECKED : MF_UNCHECKED));
    for (i = _min_scale_id - ID_SCALE; i < lengthof(SCALES); i++)
    {
        EnableMenuItem(_size_menu, ID_SCALE + i,
                       MF_BYCOMMAND | (_fullscreen ? MF_GRAYED : MF_ENABLED));
    }
}

static LRESULT CALLBACK
_wnd_proc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_CREATE: {
        _sys_menu = GetSystemMenu(wnd, FALSE);
        if (NULL != _sys_menu)
        {
            HDC     wnd_dc = GetDC(_wnd);
            float   min_scale = (float)GetDeviceCaps(wnd_dc, LOGPIXELSX) / 96.f;
            wchar_t str[MAX_PATH];
            size_t  i;

            _size_menu = CreatePopupMenu();

            ReleaseDC(_wnd, wnd_dc);

            LoadStringW(_instance, IDS_FULL, str, lengthof(str));
            AppendMenuW(_size_menu, MF_STRING, ID_FULL, str);

            AppendMenuW(_size_menu, MF_SEPARATOR, 0, NULL);

            for (i = 0; i < lengthof(SCALES); i++)
            {
                int percent;

                if (min_scale > SCALES[i])
                {
                    continue;
                }

                if (0 == _min_scale_id)
                {
                    _min_scale_id = ID_SCALE + i;
                }

                percent = (int)(SCALES[i] * 100.f / min_scale);
                wsprintfW(str, L"%d%%", percent);
                AppendMenuW(_size_menu, MF_STRING | MFS_CHECKED, ID_SCALE + i,
                            str);
            }

            CheckMenuRadioItem(_size_menu, _min_scale_id,
                               ID_SCALE + lengthof(SCALES) - 1, _min_scale_id,
                               MF_BYCOMMAND);

            AppendMenuW(_sys_menu, MF_SEPARATOR, 0, NULL);

            LoadStringW(_instance, IDS_SIZE, str, lengthof(str));
            AppendMenuW(_sys_menu, MF_POPUP, (uintptr_t)_size_menu, str);

            AppendMenuW(_sys_menu, MF_SEPARATOR, 0, NULL);

            LoadStringW(_instance, IDS_ABOUT, str, lengthof(str));
            AppendMenuW(_sys_menu, MF_STRING, ID_ABOUT, str);
        }
        break;
    }

    case WM_COMMAND:
    case WM_SYSCOMMAND: {
        if (ID_ABOUT == wparam)
        {
            const char *version = pal_get_version_string();
            wchar_t     message[1024];
            wchar_t     part[MAX_PATH];

            MultiByteToWideChar(CP_UTF8, 0, version, -1, message,
                                lengthof(message));
            _append(message, L"\n", lengthof(message));
            LoadStringW(_instance, IDS_DESCRIPTION, part, lengthof(part));
            _append(message, part, lengthof(message));
            _append(message, L"\n\n", lengthof(message));
            LoadStringW(_instance, IDS_COPYRIGHT, part, lengthof(part));
            _append(message, part, lengthof(message));
            _append(message, L"\n\nhttps://celones.pl/lavender",
                    lengthof(message));

            _hook = SetWindowsHookExW(WH_CALLWNDPROC, _about_hook_proc, NULL,
                                      GetCurrentThreadId());
            LoadStringW(_instance, IDS_ABOUT_LONG, part, lengthof(part));
            MessageBoxW(wnd, message, part, MB_ICONQUESTION);
            return 0;
        }

        if ((ID_SCALE <= LOWORD(wparam)) &&
            ((ID_SCALE + lengthof(SCALES)) > LOWORD(wparam)))
        {
            _set_scale(LOWORD(wparam) - ID_SCALE);
            return 0;
        }

        if (ID_FULL == LOWORD(wparam))
        {
            _toggle_fullscreen(wnd);
            return 0;
        }

        break;
    }

    case WM_CONTEXTMENU: {
        int      x = GET_X_LPARAM(lparam);
        int      y = GET_Y_LPARAM(lparam);
        unsigned flags = TPM_LEFTALIGN;
        if ((-1 == x) || (-1 == y))
        {
            RECT wnd_rect;
            GetWindowRect(_wnd, &wnd_rect);
            x = (wnd_rect.left + wnd_rect.right) / 2;
            y = (wnd_rect.top + wnd_rect.bottom) / 2;
            flags = TPM_CENTERALIGN | TPM_VCENTERALIGN;
        }
        TrackPopupMenu(_size_menu, flags | TPM_RIGHTBUTTON, x, y, 0, wnd, NULL);
        return 0;
    }

    case WM_KEYDOWN: {
        switch (wparam)
        {
        case VK_OEM_PLUS:
        case VK_OEM_MINUS:
        case VK_ADD:
        case VK_SUBTRACT: {
            if (0x8000 & GetKeyState(VK_CONTROL))
            {
                int scale_idx = _find_scale(
                    gfx_get_scale(),
                    ((VK_OEM_PLUS == wparam) || (VK_ADD == wparam)) ? +1 : -1);
                _set_scale(scale_idx);
                return 0;
            }

            // Fall through
        }

        case VK_F11: {
            _toggle_fullscreen(_wnd);
            return 0;
        }

        case VK_BACK:
        case VK_TAB:
        case VK_RETURN:
        case VK_ESCAPE:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_END:
        case VK_HOME:
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        case VK_INSERT:
        case VK_DELETE:
        case VK_F1:
        case VK_F2:
        case VK_F3:
        case VK_F4:
        case VK_F5:
        case VK_F6:
        case VK_F7:
        case VK_F8:
        case VK_F9:
        case VK_F10:
        case VK_F12: {
            _keycode = wparam;
            return 0;
        }

        default: {
            if ((('0' <= wparam) && ('9' >= wparam)) ||
                (('A' <= wparam) && ('Z' >= wparam)))
            {
                _keycode = wparam;
            }

            return 0;
        }
        }
    }

    case WM_KEYUP: {
        _keycode = 0;
        return 0;
    }

    case WM_MOUSEMOVE: {
        POINT origin;
        windows_get_origin(&origin);

        _mouse_x = (GET_X_LPARAM(lparam) - origin.x) / _mouse_cell.width;
        _mouse_y = (GET_Y_LPARAM(lparam) - origin.y) / _mouse_cell.height;
        return 0;
    }

    case WM_LBUTTONDOWN: {
        if (!_mouse_enabled)
        {
            return 0;
        }

        _mouse_buttons |= PAL_MOUSE_LBUTTON;
        return 0;
    }

    case WM_LBUTTONUP: {
        _mouse_buttons &= ~PAL_MOUSE_LBUTTON;
        return 0;
    }

    case WM_DESTROY:
        DestroyMenu(_size_menu);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        POINT       origin;
        HDC         dc = BeginPaint(wnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.
        HDC src_dc = windows_get_dc();

        RECT wnd_rect;
        GetClientRect(wnd, &wnd_rect);
        SetDCBrushColor(dc, windows_get_bg());
        FillRect(dc, &ps.rcPaint, GetStockObject(DC_BRUSH));

        windows_get_origin(&origin);
        BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top,
               ps.rcPaint.right - ps.rcPaint.left,
               ps.rcPaint.bottom - ps.rcPaint.top, src_dc,
               ps.rcPaint.left - origin.x, ps.rcPaint.top - origin.y, SRCCOPY);

        EndPaint(wnd, &ps);

        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wparam, lparam);
}

#if defined(_MSC_VER)
__declspec(noreturn)
#else
__attribute__((noreturn))
#endif
static void
_die(unsigned ids)
{
    WCHAR msg[MAX_PATH];
    LoadStringW(NULL, ids, msg, MAX_PATH);
    MessageBoxW(NULL, msg, L"Lavender", MB_ICONERROR);
    exit(1);
}

void
pal_initialize(int argc, char *argv[])
{
    const wchar_t wndc_name[] = L"Slideshow Window Class";
    WCHAR         title[MAX_PATH];
    WNDCLASS      wndc = {0};

    bool   arg_kiosk;
    int    i;
    hasset icon;

    _start_time = timeGetTime();
    _version = __builtin_bswap16(LOWORD(GetVersion()));

    LOG("entry");

    if (!ziparch_initialize(argv[0]))
    {
        LOG("ZIP architecture initialization failed");
        _die(IDS_NOARCHIVE);
    }

    arg_kiosk = false;
    for (i = 1; i < argc; i++)
    {
        if ('/' != argv[i][0])
        {
            continue;
        }

        if ('k' == tolower(argv[i][1]))
        {
            arg_kiosk = true;
        }
    }

    {
        INITCOMMONCONTROLSEX icc = {sizeof(INITCOMMONCONTROLSEX),
                                    ICC_STANDARD_CLASSES};
        InitCommonControlsEx(&icc);
    }

    wndc.lpfnWndProc = _wnd_proc;
    wndc.hInstance = _instance;
    wndc.lpszClassName = wndc_name;
    if (0 == RegisterClassW(&wndc))
    {
        LOG("cannot register the window class");
        _die(IDS_UNSUPPENV);
    }

    MultiByteToWideChar(CP_UTF8, 0, pal_get_version_string(), -1, title,
                        MAX_PATH);

    _wnd = CreateWindowExW(0,         // Optional window styles
                           wndc_name, // Window class
                           title,     // Window text
                           WS_OVERLAPPEDWINDOW &
                               ~(WS_MAXIMIZEBOX | WS_SIZEBOX), // Window style
                           CW_USEDEFAULT, CW_USEDEFAULT,       // Position
                           640, 480,                           // Size
                           NULL,                               // Parent
                           NULL,                               // Menu
                           _instance, // Application instance
                           NULL);
    if (NULL == _wnd)
    {
        LOG("cannot create the window");
        UnregisterClassW(wndc_name, _instance);
        _die(IDS_UNSUPPENV);
    }

    if (!gfx_initialize())
    {
        LOG("cannot initialize graphics");
        DestroyWindow(_wnd);
        UnregisterClassW(wndc_name, _instance);
        _die(IDS_UNSUPPENV);
    }

    _fullscreen = false;
    gfx_get_glyph_dimensions(&_mouse_cell);

    ShowWindow(_wnd, _cmd_show);
    pal_stall(-1);

    if (arg_kiosk)
    {
        _toggle_fullscreen(_wnd);
    }

    icon = pal_open_asset("windows.ico", O_RDONLY);
    if (icon)
    {
        int   small_size = GetSystemMetrics(SM_CYSMICON);
        int   large_size = GetSystemMetrics(SM_CYICON);
        char *data = pal_get_asset_data(icon);
        int   size = pal_get_asset_size(icon);

        int small_offset = LookupIconIdFromDirectoryEx(
            (PBYTE)data, TRUE, small_size, small_size, 0);
        int large_offset = LookupIconIdFromDirectoryEx(
            (PBYTE)data, TRUE, large_size, large_size, 0);
        _icon = NULL;

        SendMessageW(_wnd, WM_SETICON, ICON_BIG,
                     (LPARAM)CreateIconFromResource((PBYTE)data + large_offset,
                                                    size - large_offset, TRUE,
                                                    0x00030000));
        SendMessageW(_wnd, WM_SETICON, ICON_SMALL,
                     (LPARAM)CreateIconFromResource((PBYTE)data + small_offset,
                                                    size - small_offset, TRUE,
                                                    0x00030000));

        pal_close_asset(icon);
    }
    else
    {
        _icon = LoadIconW(GetModuleHandleW(NULL), MAKEINTRESOURCEW(1));
        SendMessageW(_wnd, WM_SETICON, ICON_BIG, (LPARAM)_icon);
        SendMessageW(_wnd, WM_SETICON, ICON_SMALL, (LPARAM)_icon);
    }

#if defined(CONFIG_SOUND)
    __mme_init();

    if (!snd_initialize(NULL))
    {
        LOG("cannot initialize sound");
    }
#endif // CONFIG_SOUND
}

void
pal_cleanup(void)
{
    LOG("entry");

    if (_font)
    {
        free(_font);
    }

    if (_wnd)
    {
        KillTimer(_wnd, 0);
    }

    snd_cleanup();
    gfx_cleanup();
    ziparch_cleanup();
}

bool
pal_handle(void)
{
    MSG msg;
    do
    {
        if (_no_stall && !PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            return true;
        }

        if (!_no_stall && !GetMessageW(&msg, NULL, 0, 0))
        {
            return false;
        }

        if (WM_QUIT == msg.message)
        {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    } while (_no_stall);

    return true;
}

uint16_t
pal_get_keystroke(void)
{
    uint16_t keycode = _keycode;
    _keycode = 0;
    return keycode;
}

uint32_t
pal_get_counter(void)
{
    return timeGetTime() - _start_time;
}

uint32_t
pal_get_ticks(unsigned ms)
{
    return ms;
}

void
pal_sleep(unsigned ms)
{
    LOG("entry, ms: %u", ms);

    Sleep(ms);
}

void
pal_stall(int ms)
{
    if (0 < ms)
    {
        _no_stall = false;
        SetTimer(_wnd, 0, ms, NULL);
        return;
    }

    _no_stall = (0 == ms);
    SetTimer(_wnd, 0, 20, NULL);
}

static bool
_load_version_info(void)
{
    DWORD   resource_size;
    HGLOBAL resource_data;
    LPVOID  resource;
    WORD   *translation = NULL;
    UINT    translation_len = 0;

    HRSRC resource_info = FindResourceW(NULL, MAKEINTRESOURCEW(1), RT_VERSION);
    if (NULL == resource_info)
    {
        LOG("cannot find version resource");
        return false;
    }

    resource_size = SizeofResource(NULL, resource_info);
    resource_data = LoadResource(NULL, resource_info);
    if (NULL == resource_data)
    {
        LOG("cannot load version resource");
        return false;
    }

    resource = LockResource(resource_data);
    if (NULL == resource)
    {
        LOG("cannot lock version resource");
        FreeResource(resource_data);
        return false;
    }

    _ver_resource = LocalAlloc(LMEM_FIXED, resource_size);
    if (NULL == _ver_resource)
    {
        LOG("cannot allocate memory for version resource");
        FreeResource(resource_data);
        return false;
    }

    CopyMemory(_ver_resource, resource, resource_size);
    FreeResource(resource_data);

    if (!VerQueryValueW(_ver_resource, L"\\VarFileInfo\\Translation",
                        (LPVOID *)&translation, &translation_len))
    {
        LOG("cannot query Translation");
        return false;
    }

    _ver_vfi_translation = translation;
    return true;
}

static LPCWSTR
_load_string_file_info(const char *name)
{
    WCHAR  path[MAX_PATH];
    WCHAR *string = NULL;
    UINT   string_len = 0;

    swprintf(path, MAX_PATH, L"\\StringFileInfo\\%04X%04X\\" FMT_AS,
             _ver_vfi_translation[0], _ver_vfi_translation[1], name);

    if (!VerQueryValueW(_ver_resource, path, (LPVOID *)&string, &string_len))
    {
        LOG("cannot query %s", name);
        return NULL;
    }

    return string;
}

const char *
pal_get_version_string(void)
{
    LPCWSTR name, version;
    LOG("entry");

    if (_ver_string[0])
    {
        LOG("exit, cached");
        return _ver_string;
    }

    if (!_load_version_info())
    {
        LOG("cannot load version info");
        return "Lavender";
    }

    name = _load_string_file_info("ProductName");
    if (NULL == name)
    {
        LOG("cannot load product name string");
        return "Lavender";
    }

    version = _load_string_file_info("ProductVersion");
    if (NULL == version)
    {
        LOG("cannot load product version string");
        WideCharToMultiByte(CP_UTF8, 0, name, -1, _ver_string, MAX_PATH, NULL,
                            NULL);
    }
    else
    {
        snprintf(_ver_string, MAX_PATH, "%ls %ls", name, version);
    }

    LocalFree(_ver_resource);
    LOG("exit, '%s'", _ver_string);
    return _ver_string;
}

bool
pal_get_machine_id(uint8_t *mid)
{
    char        buffer[MAX_PATH] = "";
    const char *src = buffer, *end;
    uint8_t    *dst = mid;
    HKEY        key = NULL;
    DWORD       size = sizeof(buffer);
    DWORD       type = 0;

    REGSAM wow64_key = windows_is_at_least_xp() ? KEY_WOW64_64KEY : 0;
    if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                       "SOFTWARE\\Microsoft\\Cryptography", 0,
                                       KEY_READ | wow64_key, &key))
    {
        return false;
    }

    if (ERROR_SUCCESS != RegQueryValueExA(key, "MachineGuid", NULL, &type,
                                          (LPBYTE)buffer, &size))
    {
        return false;
    }

    RegCloseKey(key);
    if (NULL == mid)
    {
        return true;
    }

    end = buffer + size;
    while ((src < end) && *src)
    {
        if (!isxdigit(*src))
        {
            src++;
            continue;
        }

        *dst = xtob(src);
        src += 2;
        dst++;
    }

    return true;
}

uint32_t
pal_get_medium_id(const char *tag)
{
    wchar_t self[MAX_PATH];
    wchar_t volume_name[MAX_PATH];
    wchar_t volume_path[MAX_PATH];
    DWORD   volume_sn = 0;
    wchar_t wide_tag[12];

    LOG("entry");

    if (0 == GetModuleFileNameW(NULL, self, MAX_PATH))
    {
        LOG("cannot retrieve executable path!");
        goto end;
    }

    if (!GetVolumePathNameW(self, volume_path, MAX_PATH))
    {
        LOG("cannot get volume path for '%ls'!", self);
        goto end;
    }

    if (!GetVolumeInformationW(volume_path, volume_name, MAX_PATH, &volume_sn,
                               NULL, NULL, NULL, 0))
    {
        LOG("cannot get volume information for '%ls'!", volume_path);
        goto end;
    }

    if (0 == MultiByteToWideChar(CP_OEMCP, 0, tag, -1, wide_tag, 12))
    {
        LOG("cannot widen tag '%s'!", tag);
        volume_sn = 0;
        goto end;
    }

    if (0 != wcscmp(wide_tag, volume_name))
    {
        LOG("volume name '%ls' not matching '%ls'!", volume_name, wide_tag);
        volume_sn = 0;
        goto end;
    }

end:
    LOG("exit, %04X-%04X", HIWORD(volume_sn), LOWORD(volume_sn));
    return (uint32_t)volume_sn;
}

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LPWSTR wbuffer;
    int    length, mb_length;

    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    wbuffer = (PWSTR)malloc(max_length * sizeof(WCHAR));
    if (NULL == wbuffer)
    {
        return -1;
    }

    length = LoadStringW(NULL, id, wbuffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);
        buffer[max_length - 1] = 0;

        LOG("exit, string missing");
        return sizeof(msg) - 1;
    }

    mb_length = WideCharToMultiByte(CP_UTF8, 0, wbuffer, length, buffer,
                                    max_length, NULL, NULL);
    buffer[mb_length] = 0;
    free(wbuffer);

    LOG("exit, '%s'", buffer);
    return length;
}

static HKEY
_open_state(void)
{
    // On Windows 7 and newer avoid placing the state in the roaming profile
    HKEY root = windows_is_at_least_7() ? HKEY_CURRENT_USER_LOCAL_SETTINGS
                                        : HKEY_CURRENT_USER;
    HKEY key = NULL;

    RegCreateKeyExA(root, "Software\\Celones\\Lavender", 0, NULL, 0,
                    KEY_ALL_ACCESS, NULL, &key, NULL);

    return key;
}

size_t
pal_load_state(const char *name, uint8_t *buffer, size_t size)
{
    DWORD byte_count = size;
    DWORD type = 0;

    HKEY key = _open_state();
    if (NULL == key)
    {
        return 0;
    }

    if (ERROR_SUCCESS !=
        RegQueryValueExA(key, name, NULL, &type, buffer, &byte_count))
    {
        byte_count = 0;
    }

    RegCloseKey(key);
    return byte_count;
}

bool
pal_save_state(const char *name, const uint8_t *buffer, size_t size)
{
    bool status = false;

    HKEY key = _open_state();
    if (NULL == key)
    {
        return status;
    }

    status = ERROR_SUCCESS ==
             RegSetValueExA(key, name, 0, REG_BINARY, buffer, (DWORD)size);

    RegCloseKey(key);
    return status;
}

void
pal_alert(const char *text, int error)
{
    WCHAR msg[MAX_PATH];
    if (error)
    {
        swprintf(msg, MAX_PATH, L"" FMT_AS L"\nerror %d", text, error);
    }
    else
    {
        MultiByteToWideChar(CP_UTF8, 0, text, -1, msg, MAX_PATH);
    }

    MessageBoxW(_wnd, msg, L"Lavender",
                error ? MB_ICONERROR : MB_ICONEXCLAMATION);
}

void
pal_open_url(const char *url)
{
    ShellExecuteA(NULL, NULL, url, NULL, NULL, SW_NORMAL);
}

HWND
windows_get_hwnd(void)
{
    return _wnd;
}

uint16_t
windows_get_version(void)
{
    return _version;
}

bool
palinet_start(void)
{
    char  user_agent[MAX_PATH] = "";
    char *ptr = NULL;

    if (NULL != _session)
    {
        return true;
    }

    strcpy(user_agent, pal_get_version_string());
    ptr = strrchr(user_agent, ' ');
    if (ptr)
    {
        *ptr = '/';
    }

    strcat(user_agent, " (");
    ptr = user_agent + strlen(user_agent);

    {
        DWORD version = GetVersion();
        if (0x80000000 & version)
        {
            ptr += sprintf(ptr, "Windows");
        }
        else if (10 == LOBYTE(version))
        {
            ptr += sprintf(ptr, "Windows NT 10.0.%u", HIWORD(version));
        }
        else
        {
            ptr += sprintf(ptr, "Windows NT %u.%u", LOBYTE(LOWORD(version)),
                           HIBYTE(LOWORD(version)));
        }
    }

    strcpy(ptr, "; ");
    ptr += 2;
    {
#if defined(_M_IX86)
        ptr += sprintf(ptr, "ia32");
#elif defined(_M_X64)
        ptr += sprintf(ptr, "x64");
#elif defined(_M_IA64)
        ptr += sprintf(ptr, "ia64");
#elif defined(_M_ARM)
        ptr += sprintf(ptr, "arm");
#elif defined(_M_ARM64)
        ptr += sprintf(ptr, "arm64");
#else
#error "Unknown architecture!"
#endif
    }

    strcpy(ptr, "; ");
    ptr += 2;
    {
        LCID locale = GetThreadLocale();
#if WINVER >= 0x0600
        WCHAR locale_name[LOCALE_NAME_MAX_LENGTH];
        if (0 <
            LCIDToLocaleName(locale, locale_name, LOCALE_NAME_MAX_LENGTH, 0))
        {
            ptr += sprintf(ptr, "%S", locale_name);
        }
#else
        switch (PRIMARYLANGID(locale))
        {
        case LANG_CZECH:
            ptr += sprintf(ptr, "cs");
            break;
        case LANG_ENGLISH:
            ptr += sprintf(ptr, "en");
            break;
        case LANG_POLISH:
            ptr += sprintf(ptr, "pl");
            break;
        default:
            ptr += sprintf(ptr, "x-lcid-%04lx", locale);
        }
#endif
    }

    strcpy(ptr, ")");

    _session = InternetOpenA(user_agent, INTERNET_OPEN_TYPE_PRECONFIG, NULL,
                             NULL, INTERNET_FLAG_ASYNC);
    return NULL != _session;
}

static char *
_format_message(DWORD error)
{
    char  *msg = NULL;
    LPWSTR wmsg = NULL;
    DWORD  length = 0;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_HMODULE,
        GetModuleHandleW(L"wininet.dll"), error, 0, (LPWSTR)&wmsg, 0, NULL);

    length = WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, NULL, 0, NULL, NULL);
    msg = (char *)malloc(length);
    if (NULL == msg)
    {
        return NULL;
    }
    WideCharToMultiByte(CP_UTF8, 0, wmsg, -1, msg, length, NULL, NULL);

    LocalFree(wmsg);
    return msg;
}

static void
_inet_error(palinet_proc *proc, DWORD error, void *data)
{
    char *msg = _format_message(error);
    proc(PALINETM_ERROR, msg, data);
    free(msg);
}

static void CALLBACK
_inet_status_callback(HINTERNET inet,
                      DWORD_PTR context,
                      DWORD     status,
                      LPVOID    status_info,
                      DWORD     status_length)
{
    switch (status)
    {
    case INTERNET_STATUS_REQUEST_COMPLETE: {
        palinet_response_param param;
        DWORD                  query_number;
        DWORD                  query_size;

        char  buffer[4096] = "";
        DWORD bytes_size = 0;

        INTERNET_ASYNC_RESULT *result = (INTERNET_ASYNC_RESULT *)status_info;
        if (ERROR_SUCCESS != result->dwError)
        {
            if (ERROR_INTERNET_OPERATION_CANCELLED == result->dwError)
            {
                break;
            }

            _inet_error(_inet_proc, result->dwError, (void *)context);
            InternetSetStatusCallbackA(_session, NULL);
            InternetCloseHandle(_request);
            InternetCloseHandle(_connection);
            _request = _connection = NULL;
            break;
        }

        query_size = sizeof(query_number);
        if (!HttpQueryInfoA(_request,
                            HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                            &query_number, &query_size, NULL))
        {
            // Cannot query status code
            _inet_error(_inet_proc, GetLastError(), (void *)context);
            break;
        }
        param.status = query_number;

        query_size = lengthof(param.status_text);
        HttpQueryInfoA(_request, HTTP_QUERY_STATUS_TEXT, param.status_text,
                       &query_size, NULL);

        query_size = sizeof(query_number);
        if (!HttpQueryInfoA(_request,
                            HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
                            &query_number, &query_size, NULL))
        {
            // Cannot query content length
            _inet_error(_inet_proc, GetLastError(), (void *)context);
            break;
        }
        param.content_length = query_number;

        _inet_proc(PALINETM_RESPONSE, &param, (void *)context);

        while (InternetReadFile(_request, buffer, sizeof(buffer) - 1,
                                &bytes_size) &&
               (0 < bytes_size))
        {
            palinet_received_param param = {bytes_size, (uint8_t *)buffer};
            buffer[bytes_size] = 0;
            if (0 != bytes_size)
            {
                _inet_proc(PALINETM_RECEIVED, &param, (void *)context);
            }
        }

        if ((ERROR_SUCCESS != GetLastError()) &&
            (ERROR_NO_MORE_ITEMS != GetLastError()))
        {
            // Reading error
            _inet_error(_inet_proc, GetLastError(), (void *)context);
            break;
        }

        _inet_proc(PALINETM_COMPLETE, NULL, (void *)context);

        InternetSetStatusCallbackA(_session, NULL);
        InternetCloseHandle(_request);
        InternetCloseHandle(_connection);
        _request = _connection = NULL;
        break;
    }
    }
}

bool
palinet_connect(const char *url, palinet_proc *proc, void *data)
{
    URL_COMPONENTSA parts;
    char            url_hostname[MAX_PATH] = "";

    if (NULL != _connection)
    {
        // Already connected
        return false;
    }

    ZeroMemory(&parts, sizeof(parts));
    parts.dwStructSize = sizeof(parts);
    parts.lpszHostName = url_hostname;
    parts.dwHostNameLength = MAX_PATH;
    if (!InternetCrackUrlA(url, 0, 0, &parts))
    {
        // URL processing error
        _inet_error(proc, GetLastError(), data);
        return false;
    }

    _inet_data = (DWORD_PTR)data;
    _inet_proc = proc;
    InternetSetStatusCallbackA(_session, _inet_status_callback);

    if (NULL == (_connection = InternetConnectA(
                     _session, parts.lpszHostName, parts.nPort, NULL, NULL,
                     INTERNET_SERVICE_HTTP, 0, _inet_data)))
    {
        // Connecting error
        _inet_error(proc, GetLastError(), data);
        return false;
    }

    proc(PALINETM_CONNECTED, NULL, data);
    return true;
}

bool
palinet_request(const char *method, const char *url)
{
    URL_COMPONENTSA parts;
    const char     *headers = NULL;
    const char     *payload = NULL;
    int             payload_length;

    if (NULL == _connection)
    {
        // Not connected
        return false;
    }

    ZeroMemory(&parts, sizeof(parts));
    parts.dwStructSize = sizeof(parts);
    parts.dwUrlPathLength = MAX_PATH;
    if (!InternetCrackUrlA(url, 0, 0, &parts))
    {
        // URL processing error
        _inet_proc(PALINETM_ERROR, NULL, (void *)_inet_data);
        return false;
    }

    if (NULL == (_request = HttpOpenRequestA(
                     _connection, method, parts.lpszUrlPath, NULL, NULL, NULL,
                     INTERNET_FLAG_NO_UI, _inet_data)))
    {
        // Request opening error
        _inet_error(_inet_proc, GetLastError(), (void *)_inet_data);
        goto end;
    }

    if (0 >
        _inet_proc(PALINETM_GETHEADERS, (void *)&headers, (void *)_inet_data))
    {
        headers = NULL;
    }

    if (0 > (payload_length = _inet_proc(PALINETM_GETPAYLOAD, (void *)&payload,
                                         (void *)_inet_data)))
    {
        payload = NULL;
    }

    if (!HttpSendRequestA(_request, headers, headers ? -1 : 0, (LPVOID)payload,
                          payload ? payload_length : 0))
    {
        if (ERROR_IO_PENDING == GetLastError())
        {
            return true;
        }

        // Sending error
        _inet_error(_inet_proc, GetLastError(), (void *)_inet_data);
        goto end;
    }

end:
    palinet_close();
    return false;
}

void
palinet_close(void)
{
    if (NULL != _request)
    {
        InternetCloseHandle(_request);
        _request = NULL;
    }

    if (NULL != _connection)
    {
        InternetCloseHandle(_connection);
        _connection = NULL;
    }
}
