#ifndef _ARD_UI_H_
#define _ARD_UI_H_

#include <sal.h>

extern int
ardui_msgbox(_In_z_ const char *str, _In_ unsigned style);

extern _Ret_z_ const char *
ardui_get_title(void);

extern void
ardui_set_title(_In_z_ const char *str);

#endif // _ARD_UI_H_
