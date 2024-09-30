#ifndef _PLATFORM_WINDOWS_H_
#define _PLATFORM_WINDOWS_H_

#ifndef WINVER
#define WINVER 0x0500
#endif
#include <windows.h>

#include <base.h>

extern HWND
windows_get_hwnd(void);

extern bool
windows_set_dialog(HWND dlg);

#if !defined(CONFIG_SDL2)
extern void
windows_set_window_title(const char *title);

extern HDC
windows_get_dc(void);

extern bool
windows_set_scale(float scale);

extern void
windows_set_box(int width, int height);

extern void
windows_get_origin(POINT *origin);
#endif

#endif // _PLATFORM_WINDOWS_H_
