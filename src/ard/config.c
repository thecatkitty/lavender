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

    // [system]
    LOAD_STRING(SEC_SYSTEM, cpu, IDS_DEFCPU);

    return &config_;
}
