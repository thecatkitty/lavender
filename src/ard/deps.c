#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include <ard/config.h>
#include <ard/version.h>

#include "resource.h"

int
ard_check_dependencies(_Inout_ ardc_config *cfg)
{
    ardc_dependency *it, *end = cfg->deps + cfg->deps_count;
    int              missing = 0;

    for (it = cfg->deps; it < end; it++)
    {
        if (it->version > ardv_dll_get_version(it->name))
        {
            missing++;
        }
        else
        {
            it->srcs_count = ARDC_DEPENDENCY_RESOLVED;
        }
    }

    return missing;
}

_Ret_maybenull_ ardc_dependency *
ard_check_sources(_Inout_ ardc_config *cfg)
{
    ardc_dependency *it, *end = cfg->deps + cfg->deps_count;

    for (it = cfg->deps; it < end; it++)
    {
        int  i;
        bool solvable = false;

        if (ARDC_DEPENDENCY_RESOLVED == it->srcs_count)
        {
            continue;
        }

        for (i = 0; !solvable && (i < it->srcs_count); i++)
        {
            if (0 <= it->srcs[i])
            {
                solvable = true;
            }
        }

        if (!solvable)
        {
            return it;
        }
    }

    return NULL;
}

static bool
might_use(_In_ const ardc_dependency *dep, _In_ int src)
{
    int i;

    if (ARDC_DEPENDENCY_RESOLVED == dep->srcs_count)
    {
        return false;
    }

    for (i = 0; i < dep->srcs_count; i++)
    {
        if (dep->srcs[i] == src)
        {
            return true;
        }
    }

    return false;
}

_Ret_maybenull_ ardc_source **
ard_get_sources(_Inout_ ardc_config *cfg)
{
    ardc_dependency *it, *end = cfg->deps + cfg->deps_count;

    ardc_source **list, **ptr;
    int          *counters;
    int           total, i;

    counters = (int *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                 cfg->srcs_count * sizeof(int));
    if (NULL == counters)
    {
        return NULL;
    }

    // for each unresolved dependency, top to bottom
    for (it = cfg->deps; it < end; it++)
    {
        ardc_dependency *xit;

        if (ARDC_DEPENDENCY_RESOLVED == it->srcs_count)
        {
            continue;
        }

        // use the source
        counters[it->srcs[0]]++;
        it->srcs[ARDC_DEPENDENCY_MAX_SOURCES] = it->srcs[0];

        // check if previous dependencies could use this source as well
        for (xit = cfg->deps; xit < it; xit++)
        {
            if (might_use(xit, it->srcs[0]))
            {
                int prev_src = xit->srcs[ARDC_DEPENDENCY_MAX_SOURCES];
                xit->srcs[ARDC_DEPENDENCY_MAX_SOURCES] = it->srcs[0];
                counters[prev_src]--;
                counters[it->srcs[0]]++;
            }
        }
    }

    total = 0;
    for (i = 0; i < cfg->srcs_count; i++)
    {
        if (0 < counters[i])
        {
            total++;
        }
    }

    list = (ardc_source **)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                      (total + 1) * sizeof(ardc_source *));
    if (NULL == list)
    {
        LocalFree(counters);
        return NULL;
    }

    ptr = list;
    for (i = 0; i < cfg->srcs_count; i++)
    {
        if (0 < counters[i])
        {
            *ptr = cfg->srcs + i;
            ptr++;
        }
    }
    *ptr = NULL;

    return list;
}
