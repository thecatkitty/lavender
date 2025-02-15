#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#if defined(_MSC_VER)
#include <intrin.h>

#define BSWAP16(x) _byteswap_ushort(x)
#else
#define BSWAP16(x) __builtin_bswap16(x)
#endif

#include <ard/version.h>

bool
ardv_windows_is_nt(void)
{
    return 0 == (GetVersion() & 0x80000000);
}

WORD
ardv_windows_get_version(void)
{
    return BSWAP16(LOWORD(GetVersion()));
}
