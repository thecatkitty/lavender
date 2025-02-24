#ifndef _ARD_ACTION_H_
#define _ARD_ACTION_H_

#include <sal.h>
#include <stdbool.h>

#include <ard/config.h>

enum
{
    ARDA_SUCCESS,
    ARDA_CONTINUE,
    ARDA_ERROR = -1,
};

extern int
arda_exec(_In_ const ardc_config *cfg, _In_z_ const char *path);

extern int
arda_run(_In_ const ardc_config *cfg);

#if defined(_M_IX86)
extern bool
arda_rundos_available(_In_ const ardc_config *cfg);
#else
#define arda_rundos_available(x) false
#endif

extern int
arda_rundos(_In_ const ardc_config *cfg);

extern int
arda_select(_In_ const ardc_config *cfg, _In_ ardc_source **sources);

extern void
arda_select_cleanup(void);

extern HWND
arda_select_get_window(void);

extern int
arda_instredist(_In_ const ardc_config *cfg, _In_ ardc_source **sources);

extern int
arda_instredist_handle(void);

#endif // _ARD_ACTION_H_
