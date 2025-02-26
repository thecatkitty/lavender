#include <sal.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#if defined(_MSC_VER)
#include <intrin.h>

#define BSWAP32(x) _byteswap_ulong(x)
#else
#define BSWAP32(x) __builtin_bswap32(x)
#endif

#include <ard/config.h>
#include <ard/version.h>

#include "resource.h"

static const char LARD_INI[] = ".\\lard.ini";
static const char SEC_LARD[] = "lard";
static const char SEC_COLORS[] = "colors";
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

static COLORREF
load_color(_In_z_ const char *section,
           _In_z_ const char *key,
           _In_ COLORREF      default_val)
{
    char     buffer[ARDC_LENGTH_SHORT], *ptr;
    COLORREF color = default_val;

    if (0 == GetPrivateProfileString(section, key, NULL, buffer,
                                     ARRAYSIZE(buffer), LARD_INI))
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
        config_.deps[i].version = parse_short_version(sep + 1);
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

    // [dependencies]
    deps_size =
        GetPrivateProfileSection(SEC_DEPS, deps, ARRAYSIZE(deps), LARD_INI);
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
