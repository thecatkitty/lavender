#ifndef _COMPAT_MINWINDEF_H_
#define _COMPAT_MINWINDEF_H_

#include <windef.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a) / sizeof(*(a)))
#endif

#endif
