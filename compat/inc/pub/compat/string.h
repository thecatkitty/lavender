#ifndef COMPAT_STRING_H
#define COMPAT_STRING_H

#include <string.h>

#if defined(__ia16__)
#include <libi86/string.h>
#else
#define _fmemcpy memcpy
#define _fmemset memset
#endif

#if defined(_MSC_VER)
#define strcasecmp _stricmp
#endif

#endif
