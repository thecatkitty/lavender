#ifndef _ARD_VERSION_H_
#define _ARD_VERSION_H_

#include <minwindef.h>
#include <sal.h>
#include <stdbool.h>

typedef enum
{
    ARDV_CPU_I386,
    ARDV_CPU_I486,
    ARDV_CPU_P5,
    ARDV_CPU_P6,
    ARDV_CPU_SSE,
    ARDV_CPU_SSE2,
    ARDV_CPU_X64,
    ARDV_CPU_MAX
} ardv_cpu_level;

extern ardv_cpu_level
ardv_cpu_get_level(void);

extern ardv_cpu_level
ardv_cpu_from_string(_In_z_ const char *str, _In_ ardv_cpu_level default_val);

extern WORD
ardv_dll_get_version(_In_z_ const char *name);

extern bool
ardv_windows_is_nt(void);

extern WORD
ardv_windows_get_version(void);

extern WORD
ardv_windows_get_servicepack(void);

extern const char *
ardv_windows_get_name(_In_ WORD version, _In_ bool is_nt);

extern const char *
ardv_windows_get_spname(_In_ WORD os_version,
                        _In_ WORD sp_version,
                        _In_ bool is_nt);

#endif // _ARD_VERSION_H_
