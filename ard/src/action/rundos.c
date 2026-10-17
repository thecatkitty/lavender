#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <ard/action.h>
#include <ard/config.h>

typedef BOOL(WINAPI *pf_iswow64process)(HANDLE, PBOOL);

#if defined(_M_IX86)
static bool available_, available_checked_ = false;

bool
arda_rundos_available(_In_ const ardc_config *cfg)
{
    HMODULE           kernel32;
    pf_iswow64process fn_isw64;
    BOOL              is_wow64 = FALSE;

    if (available_checked_)
    {
        return available_;
    }

    available_checked_ = true;
    if (0 == cfg->rundos[0])
    {
        return available_ = false;
    }

    kernel32 = GetModuleHandle("kernel32.dll");
    fn_isw64 = (pf_iswow64process)(kernel32 ? GetProcAddress(kernel32,
                                                             "IsWow64Process")
                                            : NULL);
    if (fn_isw64 && fn_isw64(GetCurrentProcess(), &is_wow64))
    {
        return available_ = !is_wow64;
    }

    return available_ = true;
}
#endif

int
arda_rundos(_In_ const ardc_config *cfg)
{
    PostQuitMessage(arda_exec(cfg, cfg->rundos));
    return 0;
}
