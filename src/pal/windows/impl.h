#ifndef _PAL_WINDOWS_IMPL_H
#define _PAL_WINDOWS_IMPL_H

#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>

#include <gfx.h>

#if defined(_MSC_VER) ||                                                       \
    (defined(__USE_MINGW_ANSI_STDIO) && !__USE_MINGW_ANSI_STDIO)
#define FMT_AS L"%S"
#else
#define FMT_AS L"%s"
#endif

extern int       windows_cmd_show;
extern HINSTANCE windows_instance;
extern HWND      windows_wnd;

extern bool  windows_fullscreen;
extern bool  windows_no_stall;
extern DWORD windows_start_time;

extern WPARAM         windows_keycode;
extern gfx_dimensions windows_cell;

extern void
windows_about(const wchar_t *title, const wchar_t *text);

extern void
windows_append(wchar_t *dst, const wchar_t *src, size_t size);

extern void
windows_toggle_fullscreen(HWND wnd);

extern LRESULT CALLBACK
windows_wndproc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam);

#endif // _PAL_WINDOWS_IMPL_H
