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

extern HDC
windows_get_dc(void);

extern HFONT
windows_find_font(int max_width, int max_height);

extern bool
windows_set_font(HFONT font);

extern bool
windows_set_scale(float scale);

extern void
windows_set_box(int width, int height);

extern void
windows_get_origin(POINT *origin);

extern COLORREF
windows_get_bg(void);

#endif // _PLATFORM_WINDOWS_H_
