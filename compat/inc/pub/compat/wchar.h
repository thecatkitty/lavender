#ifndef COMPAT_WCHAR_H
#define COMPAT_WCHAR_H

#include <wchar.h>

#if !defined(_MSC_VER) || (_MSC_VER > 1400)
#define swprintf_c swprintf
#else
extern int
swprintf_c(wchar_t *buffer, size_t bufsz, const wchar_t *format, ...);
#endif

#endif
