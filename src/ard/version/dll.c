#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <ard/version.h>

WORD
ardv_dll_get_version(_In_z_ const char *name)
{
    WORD              version = 0;
    VS_FIXEDFILEINFO *ffi;
    UINT              ffi_size;

    DWORD handle = 0;
    DWORD size = 0;
    void *data = NULL;

    if (0 == (size = GetFileVersionInfoSize(name, &handle)))
    {
        return 0;
    }

    if (NULL == (data = (void *)LocalAlloc(LMEM_FIXED, size)))
    {
        return 0;
    }

    if (!GetFileVersionInfo(name, handle, size, data) ||
        !VerQueryValue(data, "\\", (LPVOID *)&ffi, &ffi_size))
    {
        goto end;
    }

    version = min(0xFF, HIWORD(ffi->dwFileVersionMS)) << 8;
    version |= min(0xFF, LOWORD(ffi->dwFileVersionMS));

end:
    LocalFree((HLOCAL)data);
    return version;
}
