#include <arch/windows.h>
#include <pal.h>

#include "impl.h"

#ifndef HKEY_CURRENT_USER_LOCAL_SETTINGS
#define HKEY_CURRENT_USER_LOCAL_SETTINGS ((HKEY)(ULONG_PTR)((LONG)0x80000007))
#endif

static HKEY
open_state(void)
{
    // On Windows 7 and newer avoid placing the state in the roaming profile
    HKEY root = windows_is_at_least_7() ? HKEY_CURRENT_USER_LOCAL_SETTINGS
                                        : HKEY_CURRENT_USER;
    HKEY key = NULL;

    RegCreateKeyExA(root, "Software\\Celones\\Lavender", 0, NULL, 0,
                    KEY_ALL_ACCESS, NULL, &key, NULL);

    return key;
}

size_t
pal_load_state(const char *name, uint8_t *buffer, size_t size)
{
    DWORD byte_count = size;
    DWORD type = 0;

    HKEY key = open_state();
    if (NULL == key)
    {
        return 0;
    }

    if (ERROR_SUCCESS !=
        RegQueryValueExA(key, name, NULL, &type, buffer, &byte_count))
    {
        byte_count = 0;
    }

    RegCloseKey(key);
    return byte_count;
}

bool
pal_save_state(const char *name, const uint8_t *buffer, size_t size)
{
    bool status = false;

    HKEY key = open_state();
    if (NULL == key)
    {
        return status;
    }

    status = ERROR_SUCCESS ==
             RegSetValueExA(key, name, 0, REG_BINARY, buffer, (DWORD)size);

    RegCloseKey(key);
    return status;
}
