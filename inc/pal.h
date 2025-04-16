#ifndef _PAL_H_
#define _PAL_H_

#include <fcntl.h>

#include <fmt/utf8.h>

DEFINE_HANDLE(hasset);
typedef long hcache;

typedef bool (*pal_enum_assets_callback)(const char *, void *);

#ifndef PATH_MAX
#ifdef _MAX_PATH
#define PATH_MAX _MAX_PATH
#else
#define PATH_MAX MAX_PATH
#endif
#endif

#if defined(_WIN32)
#define PAL_EXTERNAL_TICK 1
#else
#define PAL_EXTERNAL_TICK 0
#endif

#define PAL_MACHINE_ID_SIZE 16

#define PAL_MOUSE_LBUTTON 0x0001
#define PAL_MOUSE_RBUTTON 0x0002

#ifndef VK_BACK
// Virtual key code definitions borrowed from Windows API
// Codes for 0-9 and A-Z are the same as ASCII
#define VK_BACK      0x08
#define VK_TAB       0x09
#define VK_RETURN    0x0D
#define VK_ESCAPE    0x1B
#define VK_PRIOR     0x21
#define VK_NEXT      0x22
#define VK_END       0x23
#define VK_HOME      0x24
#define VK_LEFT      0x25
#define VK_UP        0x26
#define VK_RIGHT     0x27
#define VK_DOWN      0x28
#define VK_INSERT    0x2D
#define VK_DELETE    0x2E
#define VK_F1        0x70
#define VK_F2        0x71
#define VK_F3        0x72
#define VK_F4        0x73
#define VK_F5        0x74
#define VK_F6        0x75
#define VK_F7        0x76
#define VK_F8        0x77
#define VK_F9        0x78
#define VK_F10       0x79
#define VK_F11       0x7A
#define VK_F12       0x7B
#define VK_OEM_MINUS 0xBD
#endif

#if defined(__linux__) || (defined(_WIN32) && defined(_DEBUG))

extern void
pal_print_log(const char *location, const char *format, ...);

#define LOG(...) pal_print_log(__func__, __VA_ARGS__)

#elif defined(_MSC_VER) && (_MSC_VER < 1500)

static __inline void
LOG(const char *fmt, ...)
{
}

#else

#define LOG(...)

#endif

extern void
pal_initialize(int argc, char *argv[]);

extern void
pal_cleanup(void);

extern bool
pal_handle(void);

#if !defined(__cplusplus) || !defined(__ia16__)

extern uint32_t ddcall
pal_get_counter(void);

extern uint32_t ddcall
pal_get_ticks(unsigned ms);

#endif

#if defined(__ia16__)

uint32_t
palpp_get_counter(void);

uint32_t
palpp_get_ticks(unsigned ms);

#else

#define palpp_get_counter pal_get_counter
#define palpp_get_ticks   pal_get_ticks

#endif

extern void ddcall
pal_sleep(unsigned ms);

#if PAL_EXTERNAL_TICK
extern void
pal_stall(int ms);
#endif

extern int
pal_enum_assets(pal_enum_assets_callback callback,
                const char              *pattern,
                void                    *data);

extern int
pal_extract_asset(const char *name, char *path);

extern hasset
pal_open_asset(const char *name, int flags);

extern bool
pal_close_asset(hasset asset);

extern bool
pal_read_asset(hasset asset, char *buff, off_t at, size_t size);

extern char *
pal_load_asset(hasset asset);

extern long
pal_get_asset_size(hasset asset);

extern hcache
pal_cache(int fd, off_t at, size_t size);

extern void
pal_read(hcache handle, char *buff, off_t at, size_t size);

extern void
pal_discard(hcache handle);

extern const char *
pal_get_version_string(void);

extern bool
pal_get_machine_id(uint8_t *mid);

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

extern size_t
pal_load_state(const char *name, uint8_t *buffer, size_t size);

extern bool
pal_save_state(const char *name, const uint8_t *buffer, size_t size);

extern void
pal_alert(const char *text, int error);

#if defined(_WIN32) || defined(__linux__)
extern void
pal_open_url(const char *url);
#endif

#ifndef UTF8_NATIVE
extern char
pal_wctoa(uint16_t wc);

extern char
pal_wctob(uint16_t wc);
#endif

#endif // _PAL_H_
