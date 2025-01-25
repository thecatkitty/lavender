#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

bool
pal_get_machine_id(uint8_t *mid)
{
    char        buffer[MAX_PATH] = "";
    const char *src = buffer, *end;
    uint8_t    *dst = mid;
    HKEY        key = NULL;
    DWORD       size = sizeof(buffer);
    DWORD       type = 0;

    REGSAM wow64_key = windows_is_at_least_xp() ? KEY_WOW64_64KEY : 0;
    if (ERROR_SUCCESS != RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                       "SOFTWARE\\Microsoft\\Cryptography", 0,
                                       KEY_READ | wow64_key, &key))
    {
        return false;
    }

    if (ERROR_SUCCESS != RegQueryValueExA(key, "MachineGuid", NULL, &type,
                                          (LPBYTE)buffer, &size))
    {
        return false;
    }

    RegCloseKey(key);
    if (NULL == mid)
    {
        return true;
    }

    end = buffer + size;
    while ((src < end) && *src)
    {
        if (!isxdigit(*src))
        {
            src++;
            continue;
        }

        *dst = xtob(src);
        src += 2;
        dst++;
    }

    return true;
}
