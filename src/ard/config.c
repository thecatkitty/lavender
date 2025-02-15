#include <sal.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <ard/config.h>
#include <ard/version.h>

#include "resource.h"

static const char LARD_INI[] = ".\\lard.ini";
static const char SEC_LARD[] = "lard";
static const char SEC_SYSTEM[] = "system";

static ardc_config config_;

static WORD
load_short_version(_In_z_ const char *section,
                   _In_z_ const char *key,
                   _In_ WORD          default_val)
{
    char buffer[ARDC_LENGTH_SHORT], *ptr;
    WORD value;
    int  part;

    if (0 == GetPrivateProfileString(section, key, NULL, buffer,
                                     ARDC_LENGTH_SHORT, LARD_INI))
    {
        return default_val;
    }

    ptr = buffer;
    part = atoi(ptr);
    value = (WORD)(min(0xFF, part) << 8);

    part = 0;
    if (NULL != (ptr = strchr(ptr, '.')))
    {
        part = atoi(++ptr);
    }
    value |= (WORD)min(0xFF, part);

    return value;
}

static void
load_string(_In_z_ const char         *section,
            _In_z_ const char         *key,
            _Out_writes_z_(size) char *buffer,
            _In_ int                   size,
            _In_ int                   default_id)
{
    if (0 != GetPrivateProfileString(section, key, NULL, buffer, (DWORD)size,
                                     LARD_INI))
    {
        return;
    }

    buffer[LoadString(NULL, default_id, buffer, size)] = 0;
}

#define LOAD_STRING(section, key, default_id)                                  \
    load_string(section, #key, config_.key, ARRAYSIZE(config_.key), default_id)

ardc_config *
ardc_load(void)
{
    // [lard]
    LOAD_STRING(SEC_LARD, name, IDS_DEFNAME);
    LOAD_STRING(SEC_LARD, run, IDS_DEFRUN);
    LOAD_STRING(SEC_LARD, rundos, IDS_DEFRUNDOS);

    // [system]
    LOAD_STRING(SEC_SYSTEM, cpu, IDS_DEFCPU);
    config_.win = load_short_version(SEC_SYSTEM, "win", 0x0400);
    config_.winnt = load_short_version(SEC_SYSTEM, "winnt", 0x0400);
    {
        char ossp_key[ARDC_LENGTH_SHORT];
        WORD version = ardv_windows_get_version();

        config_.ossp = 0;
        if (0 < sprintf(ossp_key,
                        ardv_windows_is_nt() ? "winspnt%u.%u" : "winsp%u.%u",
                        HIBYTE(version), LOBYTE(version)))
        {
            config_.ossp = load_short_version(SEC_SYSTEM, ossp_key, 0);
        }
    }

    return &config_;
}
