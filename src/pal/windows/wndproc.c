#include <math.h>
#define UNICODE
#include <windows.h>
#include <windowsx.h>

#include <arch/windows.h>
#include <pal.h>
#include <snd.h>

#include "../../resource.h"
#include "impl.h"
#include <evtmouse.h>

typedef BOOL(WINAPI *pf_getmonitorinfoa)(HMONITOR, LPMONITORINFO);
typedef HMONITOR(WINAPI *pf_monitorfromwindow)(HWND, DWORD);

#ifndef INFINITY
#define INFINITY 1000.f
#endif

#define ID_ABOUT 0x1000
#define ID_SCALE 0x2000
#define ID_FULL  0x2100

gfx_dimensions windows_cell;
bool           windows_fullscreen = false;

// See DEVICE_SCALE_FACTOR in shtypes.h
static const float SCALES[] = {1.00f, 1.20f, 1.25f, 1.40f, 1.50f, 1.60f,
                               1.75f, 1.80f, 2.00f, 2.25f, 2.50f, 3.00f,
                               3.50f, 4.00f, 4.50f, 5.00f};

static float scale_ = 0.f;
static int   scale_min_id_ = 0;

static WINDOWPLACEMENT placement_ = {sizeof(WINDOWPLACEMENT)};

static HMENU sys_menu_ = NULL;
static HMENU size_menu_ = NULL;

static int
match_scale(float scale, int direction)
{
    float best_delta = INFINITY;
    int   scale_idx = 0, i;

    if (0 == direction)
    {
        return scale_min_id_;
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

    return min(max(scale_idx + direction, scale_min_id_ - ID_SCALE),
               lengthof(SCALES) - 1);
}

static bool
select_scale(int idx)
{
    idx = min(max(scale_min_id_ - ID_SCALE, idx), lengthof(SCALES) - 1);
    if (!windows_set_scale(SCALES[idx]))
    {
        return false;
    }

    gfx_get_glyph_dimensions(&windows_cell);
    CheckMenuRadioItem(sys_menu_, scale_min_id_,
                       ID_SCALE + lengthof(SCALES) - 1, ID_SCALE + idx,
                       MF_BYCOMMAND);
    return true;
}

static bool
get_fullscreen_rect(HWND wnd, RECT *rect)
{
    MONITORINFO mi = {sizeof(mi)};

    pf_getmonitorinfoa   fn_gmi;
    pf_monitorfromwindow fn_mfw;

    if (windows_is_less_than_98())
    {
        rect->left = 0;
        rect->top = 0;
        rect->right = GetSystemMetrics(SM_CXSCREEN);
        rect->bottom = GetSystemMetrics(SM_CYSCREEN);
        return true;
    }

#if WINVER >= 0x040A
    fn_gmi = GetMonitorInfoA;
    fn_mfw = MonitorFromWindow;
#else
    fn_gmi =
        windows_get_proc("user32.dll", "GetMonitorInfoA", pf_getmonitorinfoa);
    fn_mfw = windows_get_proc("user32.dll", "MonitorFromWindow",
                              pf_monitorfromwindow);
#endif

    if (fn_gmi && fn_mfw && fn_gmi(fn_mfw(wnd, MONITOR_DEFAULTTOPRIMARY), &mi))
    {
        rect->left = mi.rcMonitor.left;
        rect->top = mi.rcMonitor.top;
        rect->right = mi.rcMonitor.right;
        rect->bottom = mi.rcMonitor.bottom;
        return true;
    }

    return false;
}

void
windows_toggle_fullscreen(HWND wnd)
{
    int i;

    DWORD style = GetWindowLongW(wnd, GWL_STYLE);
    if (windows_fullscreen)
    {
        windows_set_scale(scale_);
        SetWindowLongW(wnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(wnd, &placement_);
        SetWindowPos(wnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
    else
    {
        RECT rect;
        if (GetWindowPlacement(wnd, &placement_) &&
            get_fullscreen_rect(wnd, &rect))
        {
            int max_width = (rect.right - rect.left) / GFX_COLUMNS;
            int max_height = (rect.bottom - rect.top) / GFX_LINES;

            scale_ = gfx_get_scale();
            windows_set_font(windows_find_font(max_width, max_height));
            windows_set_box(rect.right - rect.left, rect.bottom - rect.top);
            SetWindowLongW(wnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(wnd, HWND_TOP, rect.left, rect.top,
                         rect.right - rect.left, rect.bottom - rect.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }

    gfx_get_glyph_dimensions(&windows_cell);
    windows_fullscreen = !windows_fullscreen;

    CheckMenuItem(size_menu_, ID_FULL,
                  MF_BYCOMMAND |
                      (windows_fullscreen ? MF_CHECKED : MF_UNCHECKED));
    for (i = scale_min_id_ - ID_SCALE; i < lengthof(SCALES); i++)
    {
        EnableMenuItem(size_menu_, ID_SCALE + i,
                       MF_BYCOMMAND |
                           (windows_fullscreen ? MF_GRAYED : MF_ENABLED));
    }
}

static void
init_menus(HWND wnd)
{
    HDC     wnd_dc = GetDC(wnd);
    float   min_scale = (float)GetDeviceCaps(wnd_dc, LOGPIXELSX) / 96.f;
    wchar_t str[MAX_PATH];
    size_t  i;

    sys_menu_ = GetSystemMenu(wnd, FALSE);
    if (NULL == sys_menu_)
    {
        return;
    }

    size_menu_ = CreatePopupMenu();

    ReleaseDC(wnd, wnd_dc);

    LoadStringW(windows_instance, IDS_FULL, str, lengthof(str));
    AppendMenuW(size_menu_, MF_STRING, ID_FULL, str);

    AppendMenuW(size_menu_, MF_SEPARATOR, 0, NULL);

    for (i = 0; i < lengthof(SCALES); i++)
    {
        int percent;

        if (min_scale > SCALES[i])
        {
            continue;
        }

        if (0 == scale_min_id_)
        {
            scale_min_id_ = ID_SCALE + i;
        }

        percent = (int)(SCALES[i] * 100.f / min_scale);
        wsprintfW(str, L"%d%%", percent);
        AppendMenuW(size_menu_, MF_STRING | MFS_CHECKED, ID_SCALE + i, str);
    }

    CheckMenuRadioItem(size_menu_, scale_min_id_,
                       ID_SCALE + lengthof(SCALES) - 1, scale_min_id_,
                       MF_BYCOMMAND);

    AppendMenuW(sys_menu_, MF_SEPARATOR, 0, NULL);

    LoadStringW(windows_instance, IDS_SIZE, str, lengthof(str));
    AppendMenuW(sys_menu_, MF_POPUP, (uintptr_t)size_menu_, str);

    AppendMenuW(sys_menu_, MF_SEPARATOR, 0, NULL);

    LoadStringW(windows_instance, IDS_ABOUT, str, lengthof(str));
    AppendMenuW(sys_menu_, MF_STRING, ID_ABOUT, str);
}

static void
show_context_menu(HWND wnd, int x, int y)
{
    unsigned flags = TPM_LEFTALIGN;
    if ((-1 == x) || (-1 == y))
    {
        RECT wnd_rect;
        GetWindowRect(wnd, &wnd_rect);
        x = (wnd_rect.left + wnd_rect.right) / 2;
        y = (wnd_rect.top + wnd_rect.bottom) / 2;
        flags = TPM_CENTERALIGN | TPM_VCENTERALIGN;
    }
    TrackPopupMenu(size_menu_, flags | TPM_RIGHTBUTTON, x, y, 0, wnd, NULL);
}

static void
key_down(HWND wnd, WPARAM wparam)
{
    switch (wparam)
    {
    case VK_OEM_PLUS:
    case VK_OEM_MINUS:
    case VK_ADD:
    case VK_SUBTRACT: {
        if (0x8000 & GetKeyState(VK_CONTROL))
        {
            int scale_idx = match_scale(
                gfx_get_scale(),
                ((VK_OEM_PLUS == wparam) || (VK_ADD == wparam)) ? +1 : -1);
            select_scale(scale_idx);
            break;
        }

        // Fall through
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
        windows_keycode = wparam;
        break;
    }

    case VK_F11: {
        windows_toggle_fullscreen(wnd);
        break;
    }

    default: {
        if ((('0' <= wparam) && ('9' >= wparam)) ||
            (('A' <= wparam) && ('Z' >= wparam)))
        {
            windows_keycode = wparam;
        }

        break;
    }
    }
}

static void
paint(HWND wnd)
{
    PAINTSTRUCT ps;
    POINT       origin;
    HDC         dc = BeginPaint(wnd, &ps);

    // All painting occurs here, between BeginPaint and EndPaint.
    HDC    src_dc = windows_get_dc();
    HBRUSH brush = CreateSolidBrush(windows_get_bg());

    RECT wnd_rect;
    GetClientRect(wnd, &wnd_rect);
    FillRect(dc, &ps.rcPaint, brush);

    windows_get_origin(&origin);
    BitBlt(dc, ps.rcPaint.left, ps.rcPaint.top,
           ps.rcPaint.right - ps.rcPaint.left,
           ps.rcPaint.bottom - ps.rcPaint.top, src_dc,
           ps.rcPaint.left - origin.x, ps.rcPaint.top - origin.y, SRCCOPY);

    DeleteObject(brush);
    EndPaint(wnd, &ps);
}

LRESULT CALLBACK
windows_wndproc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_CREATE: {
        init_menus(wnd);
        break;
    }

    case WM_COMMAND:
    case WM_SYSCOMMAND: {
        if (ID_ABOUT == wparam)
        {
            windows_about(NULL, NULL);
            return 0;
        }

        if ((ID_SCALE <= LOWORD(wparam)) &&
            ((ID_SCALE + lengthof(SCALES)) > LOWORD(wparam)))
        {
            select_scale(LOWORD(wparam) - ID_SCALE);
            return 0;
        }

        if (ID_FULL == LOWORD(wparam))
        {
            windows_toggle_fullscreen(wnd);
            return 0;
        }

        break;
    }

    case WM_CONTEXTMENU: {
        show_context_menu(wnd, GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        return 0;
    }

    case WM_KEYDOWN: {
        key_down(wnd, wparam);
        break;
    }

    case WM_KEYUP: {
        windows_keycode = 0;
        return 0;
    }

    case WM_MOUSEMOVE: {
        POINT origin;
        windows_get_origin(&origin);
        evtmouse_set_position(
            (GET_X_LPARAM(lparam) - origin.x) / windows_cell.width,
            (GET_Y_LPARAM(lparam) - origin.y) / windows_cell.height);
        return 0;
    }

    case WM_LBUTTONDOWN: {
        evtmouse_press(PAL_MOUSE_LBUTTON);
        return 0;
    }

    case WM_LBUTTONUP: {
        evtmouse_release(PAL_MOUSE_LBUTTON);
        return 0;
    }

    case WM_DESTROY: {
        DestroyMenu(size_menu_);
        PostQuitMessage(0);
        return 0;
    }

    case WM_PAINT: {
        paint(wnd);
        return 0;
    }

#if PAL_EXTERNAL_TICK
    case WM_TIMER: {
        // Prevents music hang during modal UI
        snd_handle();
        return 0;
    }
#endif
    }
    return DefWindowProc(wnd, msg, wparam, lparam);
}
