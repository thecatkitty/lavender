#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define UNICODE
#include <shlobj.h>
#include <windows.h>

#include <fmt/utf8.h>
#include <pal.h>
#include <platform/windows.h>
#include <snd.h>

#if defined(CONFIG_SDL2)
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <platform/sdl2arch.h>
#endif

#include "../resource.h"
#include "pal_impl.h"

#if !defined(CONFIG_SDL2)
#include <windowsx.h>

#include <dlg.h>

#include "evtmouse.h"
#endif

#if defined(_MSC_VER) || (defined(__USE_MINGW_ANSI_STDIO) && !__USE_MINGW_ANSI_STDIO)
#define FMT_AS "%S"
#else
#define FMT_AS "%s"
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

#if defined(CONFIG_SDL2)
extern SDL_Window *_window;
#endif

static HICON         _icon = NULL;
static HWND          _wnd = NULL;
static HWND          _dlg = NULL;
static char         *_font = NULL;
static LARGE_INTEGER _start_pc, _pc_freq;

#if !defined(CONFIG_SDL2)
static HINSTANCE _instance = NULL;
static int       _cmd_show;
static bool      _no_stall = false;

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
#endif

extern int
__mme_init(void);

#if !defined(CONFIG_SDL2)
extern int
main(int argc, char *argv[]);

int WINAPI
wWinMain(HINSTANCE instance,
         HINSTANCE prev_instance,
         PWSTR     cmd_line,
         int       cmd_show)
{
    _instance = instance;
    _cmd_show = cmd_show;

    char **argv = (char **)alloca((__argc + 2) * sizeof(char *));
    for (int i = 0; i < __argc; i++)
    {
        int length = WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, NULL, 0,
                                         NULL, NULL);
        argv[i] = (char *)alloca(length + 1);
        WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, argv[i], length + 1,
                            NULL, NULL);
    }
    argv[__argc] = NULL;
    return main(__argc, argv);
}

static BOOL CALLBACK
_about_enum_child_proc(HWND wnd, LPARAM lparam)
{
    wchar_t wndc_name[256];
    GetClassNameW(wnd, wndc_name, lengthof(wndc_name));
    if (0 != wcsicmp(wndc_name, L"Static"))
    {
        return TRUE;
    }

    LONG_PTR style = GetWindowLongW(wnd, GWL_STYLE);
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

        HWND icon_wnd = NULL;
        EnumChildWindows(wnd, _about_enum_child_proc, (LPARAM)&icon_wnd);
        if (NULL != icon_wnd)
        {
            HICON icon = LoadIconW(_instance, MAKEINTRESOURCEW(1));
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
    if (0 == direction)
    {
        return _min_scale_id;
    }

    int   scale_idx = 0;
    float best_delta = INFINITY;
    for (int i = 0; i < lengthof(SCALES); i++)
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
    const wchar_t *family = NULL;
    HFONT          font = NULL;

    // Determine the font family
    for (int i = 0; i < lengthof(FONT_NAMES); i++)
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

    HDC         dc = windows_get_dc();
    TEXTMETRICW metric;

    // Try the largest height
    SelectObject(dc, font);
    GetTextMetricsW(dc, &metric);
    if ((0 > max_width) || (max_width >= metric.tmAveCharWidth))
    {
        return font;
    }

    // Continue with binary search, fit width
    int min_height = 0;
    while (1 < (max_height - min_height))
    {
        DeleteObject(font);

        int height = (min_height + max_height) / 2;
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
            _window_scale = gfx_get_scale();

            int max_width = (mi.rcMonitor.right - mi.rcMonitor.left) / 80;
            int max_height = (mi.rcMonitor.bottom - mi.rcMonitor.top) / 25;
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
    for (int i = _min_scale_id - ID_SCALE; i < lengthof(SCALES); i++)
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
            wchar_t str[MAX_PATH];

            _size_menu = CreatePopupMenu();

            HDC   wnd_dc = GetDC(_wnd);
            float min_scale = (float)GetDeviceCaps(wnd_dc, LOGPIXELSX) / 96.f;
            ReleaseDC(_wnd, wnd_dc);

            LoadStringW(_instance, IDS_FULL, str, lengthof(str));
            AppendMenuW(_size_menu, MF_STRING, ID_FULL, str);

            AppendMenuW(_size_menu, MF_SEPARATOR, 0, NULL);

            for (size_t i = 0; i < lengthof(SCALES); i++)
            {
                if (min_scale > SCALES[i])
                {
                    continue;
                }

                if (0 == _min_scale_id)
                {
                    _min_scale_id = ID_SCALE + i;
                }

                int percent = (int)(SCALES[i] * 100.f / min_scale);
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
        _mouse_x = GET_X_LPARAM(lparam) / _mouse_cell.width;
        _mouse_y = GET_Y_LPARAM(lparam) / _mouse_cell.height;
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
        HDC         dc = BeginPaint(wnd, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.
        gfx_rect clip = {ps.rcPaint.left, ps.rcPaint.top,
                         ps.rcPaint.right - ps.rcPaint.left,
                         ps.rcPaint.bottom - ps.rcPaint.top};
        if (!dlg_refresh(&clip))
        {
            HDC src_dc = windows_get_dc();

            RECT wnd_rect;
            GetClientRect(wnd, &wnd_rect);
            SetDCBrushColor(dc, windows_get_bg());
            FillRect(dc, &ps.rcPaint, GetStockObject(DC_BRUSH));

            POINT origin;
            windows_get_origin(&origin);
            BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top,
                   ps.rcPaint.right - ps.rcPaint.left,
                   ps.rcPaint.bottom - ps.rcPaint.top, src_dc,
                   ps.rcPaint.left - origin.x, ps.rcPaint.top - origin.y,
                   SRCCOPY);
        }

        EndPaint(wnd, &ps);

        return 0;
    }
    }
    return DefWindowProc(wnd, msg, wparam, lparam);
}
#endif

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
    QueryPerformanceFrequency(&_pc_freq);
    QueryPerformanceCounter(&_start_pc);

    LOG("entry");

    if (!ziparch_initialize(argv[0]))
    {
        LOG("ZIP architecture initialization failed");
        _die(IDS_NOARCHIVE);
    }

#if defined(CONFIG_SDL2)
    if (!sdl2arch_initialize())
    {
        LOG("SDL2 architecture initialization failed");
        _die(IDS_UNSUPPENV);
    }

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    SDL_GetWindowWMInfo(_window, &wminfo);
    _wnd = wminfo.info.win.window;
#else
    bool arg_kiosk = false;
    for (int i = 1; i < argc; i++)
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

    INITCOMMONCONTROLSEX icc = {.dwSize = sizeof(INITCOMMONCONTROLSEX),
                                .dwICC = ICC_STANDARD_CLASSES};
    InitCommonControlsEx(&icc);

    const wchar_t wndc_name[] = L"Slideshow Window Class";
    WNDCLASS      wndc = {.lpfnWndProc = _wnd_proc,
                          .hInstance = _instance,
                          .lpszClassName = wndc_name};

    if (0 == RegisterClassW(&wndc))
    {
        LOG("cannot register the window class");
        _die(IDS_UNSUPPENV);
    }

    WCHAR title[MAX_PATH];
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
#endif

    hasset icon = pal_open_asset("windows.ico", O_RDONLY);
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

    __mme_init();

    if (!snd_initialize(NULL))
    {
        LOG("cannot initialize sound");
    }
}

void
pal_cleanup(void)
{
    LOG("entry");

    if (_font)
    {
        free(_font);
    }

#if defined(CONFIG_SDL2)
    sdl2arch_cleanup();
#else
    if (_wnd)
    {
        KillTimer(_wnd, 0);
    }
#endif
    ziparch_cleanup();
}

#if !defined(CONFIG_SDL2)
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

        if ((NULL == _dlg) || !IsDialogMessageW(_dlg, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
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
#endif

uint32_t
pal_get_counter(void)
{
    LARGE_INTEGER pc;
    QueryPerformanceCounter(&pc);
    return (pc.QuadPart - _start_pc.QuadPart) / (_pc_freq.QuadPart / 1000);
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
    HRSRC resource_info = FindResourceW(NULL, MAKEINTRESOURCEW(1), RT_VERSION);
    if (NULL == resource_info)
    {
        LOG("cannot find version resource");
        return false;
    }

    DWORD   resource_size = SizeofResource(NULL, resource_info);
    HGLOBAL resource_data = LoadResource(NULL, resource_info);
    if (NULL == resource_data)
    {
        LOG("cannot load version resource");
        return false;
    }

    LPVOID resource = LockResource(resource_data);
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

    WORD *translation;
    UINT  translation_len;
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
    WCHAR path[MAX_PATH];
    swprintf(path, MAX_PATH, L"\\StringFileInfo\\%04X%04X\\" FMT_AS,
             _ver_vfi_translation[0], _ver_vfi_translation[1], name);

    WCHAR *string;
    UINT   string_len;
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

    LPCWSTR name = _load_string_file_info("ProductName");
    if (NULL == name)
    {
        LOG("cannot load product name string");
        return "Lavender";
    }

    LPCWSTR version = _load_string_file_info("ProductVersion");
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

uint32_t
pal_get_medium_id(const char *tag)
{
    LOG("entry");
    DWORD volume_sn = 0;

    wchar_t self[MAX_PATH];
    if (0 == GetModuleFileNameW(NULL, self, MAX_PATH))
    {
        LOG("cannot retrieve executable path!");
        goto end;
    }

    wchar_t volume_path[MAX_PATH];
    if (!GetVolumePathNameW(self, volume_path, MAX_PATH))
    {
        LOG("cannot get volume path for '%ls'!", self);
        goto end;
    }

    wchar_t volume_name[MAX_PATH];
    if (!GetVolumeInformationW(volume_path, volume_name, MAX_PATH, &volume_sn,
                               NULL, NULL, NULL, 0))
    {
        LOG("cannot get volume information for '%ls'!", volume_path);
        goto end;
    }

    wchar_t wide_tag[12];
    if (0 == MultiByteToWideChar(CP_OEMCP, 0, tag, -1, wide_tag, 12))
    {
        LOG("cannot widen tag '%s'!", tag);
        goto end;
    }

    if (0 != wcscmp(wide_tag, volume_name))
    {
        LOG("volume name '%ls' not matching '%ls'!", volume_name, wide_tag);
        goto end;
    }

end:
    LOG("exit, %04X-%04X", HIWORD(volume_sn), LOWORD(volume_sn));
    return (uint32_t)volume_sn;
}

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    LPWSTR wbuffer = (PWSTR)alloca(max_length * sizeof(WCHAR));
    int    length = LoadStringW(NULL, id, wbuffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);

        LOG("exit, string missing");
        return sizeof(msg) - 1;
    }

    int mb_length = WideCharToMultiByte(CP_UTF8, 0, wbuffer, length, buffer,
                                        max_length, NULL, NULL);
    buffer[mb_length] = 0;

    LOG("exit, '%s'", buffer);
    return length;
}

#if defined(CONFIG_SDL2)
static int CALLBACK
enum_font_fam_proc(const LOGFONTW    *logfont,
                   const TEXTMETRICW *metric,
                   DWORD              type,
                   LPARAM             lparam)
{
    if ((TRUETYPE_FONTTYPE != type) ||
        (FIXED_PITCH != (logfont->lfPitchAndFamily & 0x3)) ||
        (FW_NORMAL != logfont->lfWeight))
    {
        return 1;
    }

    *(bool *)lparam = true;
    return 1;
}

const char *
sdl2arch_get_font(void)
{
    if (_font)
    {
        return _font;
    }

    HKEY hkfonts;
    if (ERROR_SUCCESS !=
        RegOpenKeyW(HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts",
                    &hkfonts))
    {
        LOG("cannot retrive fonts registry key");
        return NULL;
    }

    HDC   hdc = GetDC(NULL);
    bool  found = false;
    DWORD index = 0;
    WCHAR value[MAX_PATH];
    DWORD value_len = MAX_PATH;
    DWORD type;
    BYTE  data[MAX_PATH * sizeof(WCHAR)];
    DWORD data_size = sizeof(data);
    while (ERROR_SUCCESS == RegEnumValueW(hkfonts, index, value, &value_len,
                                          NULL, &type, data, &data_size))
    {
        index++;
        value_len = MAX_PATH;
        data_size = sizeof(data);

        if (REG_SZ != type)
        {
            continue;
        }

        LPWSTR ttf_suffix = wcsstr(value, L" (TrueType)");
        if (ttf_suffix)
        {
            *ttf_suffix = 0;
        }

        EnumFontFamiliesW(hdc, value, enum_font_fam_proc, (LPARAM)&found);
        if (found)
        {
            LOG("font: '%ls', file: '%ls'", value, data);
            break;
        }
    }

    RegCloseKey(hkfonts);

    if (!found)
    {
        LOG("cannot match font");
        return NULL;
    }

    WCHAR path[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT,
                                    path)))
    {
        LOG("cannot retrieve font directory path");
        return NULL;
    }

    wcsncat(path, L"\\", MAX_PATH);
    wcsncat(path, (LPWSTR)data, MAX_PATH);

    int length = WideCharToMultiByte(CP_UTF8, 0, path, -1, NULL, 0, NULL, NULL);
    _font = (char *)malloc(length + 1);
    if (NULL == _font)
    {
        LOG("cannot allocate buffer");
        return NULL;
    }

    WideCharToMultiByte(CP_UTF8, 0, path, -1, _font, length, NULL, NULL);
    return _font;
}
#endif

void
pal_alert(const char *text, int error)
{
    WCHAR msg[MAX_PATH];
    if (error)
    {
        swprintf(msg, MAX_PATH, L"" FMT_AS "\nerror %d", text, error);
    }
    else
    {
        MultiByteToWideChar(CP_UTF8, 0, text, -1, msg, MAX_PATH);
    }

    MessageBoxW(_wnd, msg, L"Lavender",
                error ? MB_ICONERROR : MB_ICONEXCLAMATION);
}

HWND
windows_get_hwnd(void)
{
    return _wnd;
}

bool
windows_set_dialog(HWND dlg)
{
    if (_dlg == dlg)
    {
        return true;
    }

    if ((NULL == _dlg) || (NULL == dlg))
    {
        _no_stall = false;
        _dlg = dlg;
        return true;
    }

    return false;
}

#if !defined(CONFIG_SDL2)
void
windows_set_window_title(const char *title)
{
    WCHAR wtitle[MAX_PATH];
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, MAX_PATH);
    SetWindowTextW(_wnd, wtitle);
}
#endif

#if defined(CONFIG_SDL2)
#define WINAPISHIM(dll, name, type, args, body)                                \
    type __stdcall _imp__##name args                                           \
    {                                                                          \
        static type(__stdcall *_pfn) args = NULL;                              \
        if (NULL == _pfn)                                                      \
        {                                                                      \
            HMODULE hmo = LoadLibraryW(L##dll);                                \
            if (hmo)                                                           \
            {                                                                  \
                _pfn = (type(__stdcall *) args)GetProcAddress(hmo, #name);     \
            }                                                                  \
        }                                                                      \
                                                                               \
        body;                                                                  \
    }

#define WINAPICALL(...)                                                        \
    if (_pfn)                                                                  \
    {                                                                          \
        return _pfn(__VA_ARGS__);                                              \
    }

WINAPISHIM("kernel32.dll", AttachConsole, BOOL, (DWORD dwProcessId), {
    WINAPICALL(dwProcessId);

    return FALSE;
})

WINAPISHIM("kernel32.dll",
           GetModuleHandleExW,
           BOOL,
           (DWORD dwFlags, LPCWSTR lpModuleName, HMODULE *phModule),
           {
               WINAPICALL(dwFlags, lpModuleName, phModule);

               if (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS & dwFlags)
               {
                   *phModule = GetModuleHandleW(NULL);
                   return TRUE;
               }

               if (GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT & dwFlags)
               {
                   *phModule = GetModuleHandleW(lpModuleName);
                   return TRUE;
               }

               return FALSE;
           })

WINAPISHIM("user32.dll",
           GetRawInputData,
           UINT,
           (HRAWINPUT hRawInput,
            UINT      uiCommand,
            LPVOID    pData,
            PUINT     pcbSize,
            UINT      cbSizeHeader),
           {
               WINAPICALL(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

               return -1;
           })

WINAPISHIM("user32.dll",
           GetRawInputDeviceInfoA,
           UINT,
           (HANDLE hDevice, UINT uiCommand, LPVOID pData, PUINT pcbSize),
           {
               WINAPICALL(hDevice, uiCommand, pData, pcbSize);

               return -1;
           })

WINAPISHIM("user32.dll",
           GetRawInputDeviceList,
           UINT,
           (PRAWINPUTDEVICELIST pRawInputDeviceList,
            PUINT               puiNumDevices,
            UINT                cbSize),
           {
               WINAPICALL(pRawInputDeviceList, puiNumDevices, cbSize);

               return -1;
           })

WINAPISHIM("user32.dll",
           RegisterRawInputDevices,
           BOOL,
           (PCRAWINPUTDEVICE pRawInputDevices, UINT uiNumDevices, UINT cbSize),
           {
               WINAPICALL(pRawInputDevices, uiNumDevices, cbSize);

               return FALSE;
           })
#endif
