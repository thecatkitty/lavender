#include <windows.h>

#include <ard/version.h>

typedef BOOL(WINAPI *pf_isprocessorfeaturepresent)(DWORD);

static const char *CPU_LEVELS[ARDV_CPU_MAX] = {"i386", "i486", "p5", "p6",
                                               "sse",  "sse2", "x64"};

ardv_cpu_level
ardv_cpu_get_level(void)
{
#if defined(_M_IX86)
    HMODULE                      kernel32 = NULL;
    pf_isprocessorfeaturepresent fn_ispfp = NULL;

    SYSTEM_INFO info;
    GetSystemInfo(&info);
#endif

#if defined(_M_IX86)
    switch (info.dwProcessorType)
    {
    case PROCESSOR_INTEL_386:
        return ARDV_CPU_I386;
    case PROCESSOR_INTEL_486:
        return ARDV_CPU_I486;
    case PROCESSOR_INTEL_PENTIUM:
        if (!ardv_windows_is_nt())
        {
            return ARDV_CPU_P5;
        }
    }

    if (6 > info.wProcessorLevel)
    {
        return ARDV_CPU_P5;
    }

    kernel32 = GetModuleHandle("kernel32.dll");
    fn_ispfp =
        (pf_isprocessorfeaturepresent)(kernel32
                                           ? GetProcAddress(
                                                 kernel32,
                                                 "IsProcessorFeaturePresent")
                                           : NULL);

    if (fn_ispfp && fn_ispfp(PF_XMMI64_INSTRUCTIONS_AVAILABLE))
    {
        return ARDV_CPU_SSE2;
    }

    if (fn_ispfp && fn_ispfp(PF_XMMI_INSTRUCTIONS_AVAILABLE))
    {
        return ARDV_CPU_SSE;
    }

    return ARDV_CPU_P6;
#elif defined(_M_X64)
    return ARDV_CPU_X64;
#elif defined(_M_ALPHA)
    return ARDV_CPU_I486;
#else
#error "Unknown architecture!"
#endif
}

ardv_cpu_level
ardv_cpu_from_string(_In_z_ const char *str, _In_ ardv_cpu_level default_val)
{
    ardv_cpu_level i;

    for (i = 0; i < ARDV_CPU_MAX; i++)
    {
        if (0 == strcmp(CPU_LEVELS[i], str))
        {
            return i;
        }
    }

    return default_val;
}
