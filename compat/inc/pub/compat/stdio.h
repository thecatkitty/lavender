#ifndef COMPAT_STDIO_H
#define COMPAT_STDIO_H

#include <stdio.h>

#if defined(_MSC_VER) && (_MSC_VER < 1800)
#define snprintf _snprintf
#endif

#endif
