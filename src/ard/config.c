#include <sal.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include <ard/config.h>
#include <ard/version.h>

#include "resource.h"

WINBASEAPI LCID WINAPI
GetThreadLocale(void);

#define LARD_INI "lard.ini"

static const char SEC_LARD[] = "lard";
static const char SEC_COLORS[] = "colors";
static const char SEC_SYSTEM[] = "system";
static const char SEC_IE[] = "ie";
static const char SEC_DEPS[] = "dependencies";

static ardc_config config_;
static char        global_ini_[MAX_PATH] = "";
static char        local_ini_[MAX_PATH] = "";
static char        root_[MAX_PATH] = "";

static WORD
get_profile_string(_In_z_ const char      *section,
                   _In_z_ const char      *key,
                   _Out_z_cap_(size) char *buffer,
                   _In_ DWORD              size)
{
    WORD length;

    if (0 == global_ini_[0])
    {
        sprintf(global_ini_, "%s" LARD_INI, ardc_get_root());
        sprintf(local_ini_, "%s%04x\\" LARD_INI, ardc_get_root(),
                LOWORD(GetThreadLocale()));
    }

    if (0 != (length = GetPrivateProfileString(section, key, "", buffer, size,
                                               local_ini_)))
    {
        return length;
    }

    return GetPrivateProfileString(section, key, "", buffer, size, global_ini_);
}

DWORD
ardc_parse_version(_In_z_ const char *str)
{
    DWORD value;
    int   part;

    part = atoi(str);
    value = (DWORD)(min(0xFF, part) << 24);

    part = 0;
    if (NULL != (str = strchr(str, '.')))
    {
        part = atoi(++str);
    }
    value |= (DWORD)(min(0xFF, part) << 16);

    part = 0;
    if (str && (NULL != (str = strchr(str, '.'))))
    {
        part = atoi(++str);
    }
    value |= (DWORD)min(0xFFFF, part);

    return value;
}

static DWORD
load_long_version(_In_z_ const char *section,
                  _In_z_ const char *key,
                  _In_ DWORD         default_val)
{
    char buffer[ARDC_LENGTH_SHORT];

    if (0 == get_profile_string(section, key, buffer, ARDC_LENGTH_SHORT))
    {
        return default_val;
    }

    return ardc_parse_version(buffer);
}

static WORD
load_short_version(_In_z_ const char *section,
                   _In_z_ const char *key,
                   _In_ WORD          default_val)
{
    return HIWORD(load_long_version(section, key, default_val << 16));
}

static void
load_string(_In_z_ const char         *section,
            _In_z_ const char         *key,
            _Out_writes_z_(size) char *buffer,
            _In_ int                   size,
            _In_ int                   default_id)
{
    if (0 != get_profile_string(section, key, buffer, (DWORD)size))
    {
        return;
    }

    buffer[LoadString(NULL, default_id, buffer, size)] = 0;
}

#define LOAD_STRING(section, key, default_id)                                  \
    load_string(section, #key, config_.key, ARRAYSIZE(config_.key), default_id)

static COLORREF
load_color(_In_z_ const char *section,
           _In_z_ const char *key,
           _In_ COLORREF      default_val)
{
    char     buffer[ARDC_LENGTH_SHORT], *ptr;
    COLORREF color = default_val;

    if (0 == get_profile_string(section, key, buffer, ARRAYSIZE(buffer)))
    {
        return color;
    }

    if ('#' != buffer[0])
    {
        return color;
    }

    color = 0;
    ptr = buffer + 1;
    while (*ptr)
    {
        int digit = ('9' < *ptr) ? (tolower(*ptr) - 'a' + 10) : (*ptr - '0');
        color <<= 4;
        color |= digit;
        ptr++;
    }

    return BSWAP32(color << 8);
}

#define LOAD_COLOR(section, key, default_val)                                  \
    config_.key##_color = load_color(section, #key, default_val)

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
        config_.deps[i].version = HIWORD(ardc_parse_version(sep + 1));
        config_.deps[i].srcs_count = 0;
        memset(config_.deps[i].srcs, 0xFF, sizeof(config_.deps[i].srcs));
        str = sep + strlen(sep) + 1;
        i++;
    }

    return true;
}

static int
load_source(_In_z_ const char *name)
{
    char section[7 + ARDC_LENGTH_SHORT] = "source.";
    int  idx;

    for (idx = 0; idx < config_.srcs_count; idx++)
    {
        if (0 == strcmp(config_.srcs[idx].name, name))
        {
            return idx;
        }
    }

    strncat(section, name, ARDC_LENGTH_SHORT);
    load_string(section, "path", config_.srcs[idx].path,
                ARRAYSIZE(config_.srcs[idx].path), IDS_DEFRUNDOS);
    if (0 == config_.srcs[idx].path[0])
    {
        return -1;
    }

    load_string(section, "description", config_.srcs[idx].description,
                ARRAYSIZE(config_.srcs[idx].description), IDS_DEFRUNDOS);
    if (0 == config_.srcs[idx].description[0])
    {
        char *begin, *end;
        char  saved;

        // extract the base name from the path
        end = strchr(config_.srcs[idx].path, ' ');
        end = end ? end : strchr(config_.srcs[idx].path, 0);

        saved = *end;
        *end = 0;
        begin = strrchr(config_.srcs[idx].path, '\\');
        *end = saved;
        begin = begin ? (begin + 1) : config_.srcs[idx].path;

        memcpy(config_.srcs[idx].description, begin, end - begin);
        *end = saved;
    }

    strcpy(config_.srcs[idx].name, name);
    config_.srcs_count++;
    return idx;
}

static void
load_sources(_In_z_ char *deps)
{
    char  *str = deps;
    size_t i;

    int max_sources = config_.deps_count * ARDC_DEPENDENCY_MAX_SOURCES;
    config_.srcs = (ardc_source *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                             max_sources * sizeof(ardc_source));
    if (NULL == config_.srcs)
    {
        // lack of sources is nonfatal - we'll just inform the user
        return;
    }

    // for each item in the dependency list
    i = 0;
    while (*str && (i < config_.deps_count))
    {
        ardc_dependency *dep = config_.deps + i;
        char            *part = str;

        // populate the sources list of the current dependency
        while ((NULL != (part = strchr(part, ','))) &&
               (ARDC_DEPENDENCY_MAX_SOURCES > dep->srcs_count))
        {
            char  saved;
            char *end;

            // extract the trimmed source name
            part += 1 + strspn(part + 1, " ");
            if (0 == *part)
            {
                continue;
            }

            end = strchr(part, ',');
            end = end ? end : (part + strlen(part));

            // add a source index to the dependency
            // possibly loading the source description as well
            saved = *end;
            *end = 0;
            dep->srcs[dep->srcs_count++] = load_source(part);
            *end = saved;
        }

        str += strlen(str) + 1;
        i++;
    }

    // free the sources list if empty
    if (0 == config_.srcs_count)
    {
        LocalFree(config_.srcs);
        config_.srcs = NULL;
        return;
    }

    // resize the sources list
    if (max_sources > config_.srcs_count)
    {
        HLOCAL srcs = LocalReAlloc(
            config_.srcs, config_.srcs_count * sizeof(ardc_source), LMEM_FIXED);
        if (srcs)
        {
            config_.srcs = srcs;
        }
    }
}

ardc_config *
ardc_load(void)
{
    char  deps[ARDC_LENGTH_MID * 5];
    DWORD deps_size;

    ZeroMemory(&config_, sizeof(config_));

    // [lard]
    LOAD_STRING(SEC_LARD, name, IDS_DEFNAME);
    LOAD_STRING(SEC_LARD, run, IDS_DEFRUN);
    LOAD_STRING(SEC_LARD, rundos, IDS_DEFRUNDOS);
    LOAD_STRING(SEC_LARD, copyright, IDS_DEFCOPYRIGHT);

    // [colors]
    LOAD_COLOR(SEC_COLORS, intro, 0xFFFFFF);
    LOAD_COLOR(SEC_COLORS, title, 0xFFFFFF);
    LOAD_COLOR(SEC_COLORS, text, 0xFFFFFF);
    LOAD_COLOR(SEC_COLORS, footer, 0xFFFFFF);

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

    // [ie]
    config_.ie_complete = load_long_version(SEC_IE, "complete", 0);
    config_.ie_offer = load_long_version(SEC_IE, "offer", 0);
    get_profile_string(SEC_IE, "description", config_.ie_description,
                       ARRAYSIZE(config_.ie_description));
    get_profile_string(SEC_IE, "install", config_.ie_install,
                       ARRAYSIZE(config_.ie_install));

    // [dependencies]
    deps_size =
        GetPrivateProfileSection(SEC_DEPS, deps, ARRAYSIZE(deps), global_ini_);
    if (((ARRAYSIZE(deps) - 2) == deps_size) || !load_dependencies(deps))
    {
        return NULL;
    }

    // [sources.*] pointed to by [dependencies]
    load_sources(deps);

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

    if (0 < config_.srcs_count)
    {
        LocalFree(config_.srcs);
        config_.srcs = NULL;
        config_.srcs_count = 0;
    }
}

const char *
ardc_get_root(void)
{
    char *ptr;

    if (0 != root_[0])
    {
        return root_;
    }

    GetModuleFileName(NULL, root_, ARRAYSIZE(root_));
    ptr = strrchr(root_, '\\');
    if (ptr)
    {
        ptr[1] = 0;
        return root_;
    }

    return strcpy(root_, ".\\");
}
