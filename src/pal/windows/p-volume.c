#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

uint32_t
pal_get_medium_id(const char *tag)
{
    wchar_t self[MAX_PATH];
    wchar_t volume_name[MAX_PATH];
    wchar_t volume_path[MAX_PATH];
    DWORD   volume_sn = 0;
    wchar_t wide_tag[12];

    LOG("entry");

    if (0 == GetModuleFileNameW(NULL, self, MAX_PATH))
    {
        LOG("cannot retrieve executable path!");
        goto end;
    }

    if (!GetVolumePathNameW(self, volume_path, MAX_PATH))
    {
        LOG("cannot get volume path for '%ls'!", self);
        goto end;
    }

    if (!GetVolumeInformationW(volume_path, volume_name, MAX_PATH, &volume_sn,
                               NULL, NULL, NULL, 0))
    {
        LOG("cannot get volume information for '%ls'!", volume_path);
        goto end;
    }

    if (0 == MultiByteToWideChar(CP_OEMCP, 0, tag, -1, wide_tag, 12))
    {
        LOG("cannot widen tag '%s'!", tag);
        volume_sn = 0;
        goto end;
    }

    if (0 != wcscmp(wide_tag, volume_name))
    {
        LOG("volume name '%ls' not matching '%ls'!", volume_name, wide_tag);
        volume_sn = 0;
        goto end;
    }

end:
    LOG("exit, %04X-%04X", HIWORD(volume_sn), LOWORD(volume_sn));
    return (uint32_t)volume_sn;
}
