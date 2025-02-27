#define WIN32_LEAN_AND_MEAN

#include <stdlib.h>
#include <windows.h>

#include <ard/config.h>
#include <ard/version.h>

bool
ardv_ie_available(void)
{
    bool ret = false;
    HKEY key;

    if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                      "SOFTWARE\\Microsoft\\Internet Explorer",
                                      0, KEY_READ, &key))
    {
        return false;
    }

    if (ERROR_SUCCESS ==
        RegQueryValueEx(key, "Version", NULL, NULL, NULL, NULL))
    {
        ret = true;
    }

    if (!ret &&
        (ERROR_SUCCESS == RegQueryValueEx(key, "IVer", NULL, NULL, NULL, NULL)))
    {
        ret = true;
    }

    RegCloseKey(key);
    return ret;
}

DWORD
ardv_ie_get_version(void)
{
    ardv_version_info info;
    DWORD             version = 0;

    if (!ardv_dll_get_version_info("url.dll", &info))
    {
        return version;
    }

    version |= info.major << 24;
    version |= info.minor << 16;
    version |= info.build ? info.build : info.patch;
    return version;
}
