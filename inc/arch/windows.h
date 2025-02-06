#ifndef _ARCH_WINDOWS_H_
#define _ARCH_WINDOWS_H_

#define UNICODE
#include <windows.h>

#include <base.h>
#include <gfx.h>

extern HWND
windows_get_hwnd(void);

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

extern HBITMAP
windows_create_dib(HDC dc, gfx_bitmap *bm);

extern uint16_t
windows_get_version(void);

#if defined(_MSC_VER) && (_MSC_VER < 1800)
#define inline __inline
#endif

inline static bool
windows_is_at_least(uint16_t ver)
{
    return ver <= windows_get_version();
}

inline static bool
windows_is_less_than(uint16_t ver)
{
    return ver > windows_get_version();
}

#define winver_or_windows_is_at_least(ver)                                     \
    ((WINVER >= (ver)) || windows_is_at_least((ver)))

#define winver_and_windows_is_less_than(ver)                                   \
    ((WINVER < (ver)) && windows_is_less_than((ver)))

#define windows_is_at_least_xp()    winver_or_windows_is_at_least(0x0501)
#define windows_is_at_least_vista() winver_or_windows_is_at_least(0x0600)
#define windows_is_at_least_7()     winver_or_windows_is_at_least(0x0601)

#define windows_is_less_than_2000() winver_and_windows_is_less_than(0x0500)

#define windows_get_proc(module, name, type)                                   \
    (GetModuleHandleA(module)                                                  \
         ? (type)GetProcAddress(GetModuleHandleA(module), name)                \
         : NULL)

#endif // _ARCH_WINDOWS_H_
