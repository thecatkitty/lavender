#ifndef _PLATFORM_WINDOWS_H_
#define _PLATFORM_WINDOWS_H_

#ifndef WINVER
#define WINVER 0x0500
#endif
#include <windows.h>

extern HWND
windows_get_hwnd(void);

#endif // _PLATFORM_WINDOWS_H_
