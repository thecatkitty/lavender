#ifndef _PAL_H_
#define _PAL_H_

#include <fcntl.h>

#include <base.h>

DEFINE_HANDLE(hasset);

#define PAL_MOUSE_LBUTTON 0x0001
#define PAL_MOUSE_RBUTTON 0x0002

#ifndef VK_BACK
// Virtual key code definitions borrowed from Windows API
// Codes for 0-9 and A-Z are the same as ASCII
#define VK_BACK   0x08
#define VK_TAB    0x09
#define VK_RETURN 0x0D
#define VK_ESCAPE 0x1B
#define VK_PRIOR  0x21
#define VK_NEXT   0x22
#define VK_END    0x23
#define VK_HOME   0x24
#define VK_LEFT   0x25
#define VK_UP     0x26
#define VK_RIGHT  0x27
#define VK_DOWN   0x28
#define VK_INSERT 0x2D
#define VK_DELETE 0x2E
#define VK_F1     0x70
#define VK_F2     0x71
#define VK_F3     0x72
#define VK_F4     0x73
#define VK_F5     0x74
#define VK_F6     0x75
#define VK_F7     0x76
#define VK_F8     0x77
#define VK_F9     0x78
#define VK_F10    0x79
#define VK_F11    0x7A
#define VK_F12    0x7B
#endif

#if defined(__linux__) || defined(__MINGW32__)

#include <stdio.h>

extern void
__pal_log_time(void);

#define LOG(fmt, ...)                                                          \
    {                                                                          \
        __pal_log_time();                                                      \
        fprintf(stderr, "%s: ", __FUNCTION__);                                 \
        fprintf(stderr, fmt "\n", ##__VA_ARGS__);                              \
    }

#else

#define LOG(fmt, ...)

#endif

extern void
pal_initialize(int argc, char *argv[]);

extern void
pal_cleanup(void);

extern void
pal_handle(void);

extern uint32_t
pal_get_counter(void);

extern uint32_t
pal_get_ticks(unsigned ms);

extern void
pal_sleep(unsigned ms);

extern hasset
pal_open_asset(const char *name, int flags);

extern bool
pal_close_asset(hasset asset);

extern char *
pal_get_asset_data(hasset asset);

extern int
pal_get_asset_size(hasset asset);

extern const char *
pal_get_version_string(void);

extern uint32_t
pal_get_medium_id(const char *tag);

extern uint16_t
pal_get_keystroke(void);

extern void
pal_enable_mouse(void);

extern void
pal_disable_mouse(void);

extern uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y);

extern int
pal_load_string(unsigned id, char *buffer, int max_length);

#endif // _PAL_H_
