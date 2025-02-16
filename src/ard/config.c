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
static const char SEC_DEPS[] = "dependencies";

static ardc_config config_;

static WORD
parse_short_version(_In_z_ const char *str)
{
    WORD value;
    int  part;

    part = atoi(str);
    value = (WORD)(min(0xFF, part) << 8);

    part = 0;
    if (NULL != (str = strchr(str, '.')))
    {
        part = atoi(++str);
    }
    value |= (WORD)min(0xFF, part);

    return value;
}

static WORD
load_short_version(_In_z_ const char *section,
                   _In_z_ const char *key,
                   _In_ WORD          default_val)
{
    char buffer[ARDC_LENGTH_SHORT];

    if (0 == GetPrivateProfileString(section, key, NULL, buffer,
                                     ARDC_LENGTH_SHORT, LARD_INI))
    {
        return default_val;
    }

    return parse_short_version(buffer);
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

static size_t
count_values(_In_z_ const char *values)
{
    size_t ret = 0;

    while (*values)
    {
        while (*(values++))
            ;

        ret++;
    }

    return ret;
}

static bool
load_dependencies(_In_z_ const char *deps)
{
    const char *str = deps;
    size_t      i;

    if (0 == (config_.deps_count = count_values(deps)))
    {
        return true;
    }

    config_.deps = (ardc_dependency *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                                 config_.deps_count *
                                                     sizeof(ardc_dependency));
    if (NULL == config_.deps)
    {
        return false;
    }

    i = 0;
    while (*str && (i < config_.deps_count))
    {
        const char *sep = strchr(str, '=');
        strncpy(config_.deps[i].name, str, sep - str);
        config_.deps[i].version = parse_short_version(sep + 1);
        str = sep + strlen(sep) + 1;
        i++;
    }

    return true;
}

ardc_config *
ardc_load(void)
{
    char  deps[ARDC_LENGTH_MID * 5];
    DWORD deps_size;

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

    // [dependencies]
    deps_size =
        GetPrivateProfileSection(SEC_DEPS, deps, ARRAYSIZE(deps), LARD_INI);
    if (((ARRAYSIZE(deps) - 2) == deps_size) || !load_dependencies(deps))
    {
        return NULL;
    }

    return &config_;
}

void
ardc_cleanup(void)
{
    if (0 < config_.deps_count)
    {
        LocalFree(config_.deps);
        config_.deps = NULL;
        config_.deps_count = 0;
    }
}
