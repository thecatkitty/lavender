#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <ard/version.h>

_Success_(return == true) bool ardv_dll_get_version_info(
    _In_z_ const char *name, _Out_ ardv_version_info *info)
{
    VS_FIXEDFILEINFO *ffi = NULL;
    UINT              ffi_size;

    DWORD handle = 0;
    DWORD size = 0;
    void *data = NULL;
    bool  status = true;

    if (0 == (size = GetFileVersionInfoSize(name, &handle)))
    {
        return false;
    }

    if (NULL == (data = (void *)LocalAlloc(LMEM_FIXED, size)))
    {
        return false;
    }

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 6388) // handle can be non-NULL under Windows 9x
#endif

    if (!GetFileVersionInfo(name, handle, size, data) ||
        !VerQueryValue(data, "\\", (LPVOID *)&ffi, &ffi_size))
    {
        status = false;
        goto end;
    }

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

    info->major = min(MAXBYTE, HIWORD(ffi->dwFileVersionMS));
    info->minor = min(MAXBYTE, LOWORD(ffi->dwFileVersionMS));
    info->build = HIWORD(ffi->dwFileVersionLS);
    info->patch = LOWORD(ffi->dwFileVersionLS);

end:
    LocalFree((HLOCAL)data);
    return status;
}

WORD
ardv_dll_get_version(_In_z_ const char *name)
{
    ardv_version_info info;

    if (!ardv_dll_get_version_info(name, &info))
    {
        return 0;
    }

    return (info.major << 8) | info.minor;
}
