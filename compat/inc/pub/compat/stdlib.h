#ifndef COMPAT_STDLIB_H
#define COMPAT_STDLIB_H

#include <stdlib.h>

#if defined(__ia16__)
#include <libi86/malloc.h>
#else
#define _ffree   free
#define _fmalloc malloc
#endif

#if !defined(_WIN32) && !__MISC_VISIBLE
#define CUSTOM_ITOA
extern char *
itoa(int value, char *str, int base);
#endif

#endif
