#ifndef _ARD_CONFIG_H_
#define _ARD_CONFIG_H_

#include <minwindef.h>
#include <stdbool.h>
#include <stdint.h>

#define ARDC_LENGTH_LONG  MAX_PATH
#define ARDC_LENGTH_MID   80
#define ARDC_LENGTH_SHORT 20

#define ARDC_DEPENDENCY_MAX_SOURCES 3
#define ARDC_DEPENDENCY_RESOLVED    -1

typedef struct
{
    char name[ARDC_LENGTH_SHORT];
    char description[ARDC_LENGTH_MID];
    char path[ARDC_LENGTH_LONG];
} ardc_source;

typedef struct
{
    char     name[ARDC_LENGTH_SHORT];
    uint16_t version;
    int      srcs_count;
    int      srcs[ARDC_DEPENDENCY_MAX_SOURCES + 1];
} ardc_dependency;

typedef struct
{
    // [lard]
    char name[ARDC_LENGTH_MID];
    char run[ARDC_LENGTH_LONG];
    char rundos[ARDC_LENGTH_LONG];
    char copyright[ARDC_LENGTH_LONG];

    // [colors]
    COLORREF intro_color;
    COLORREF title_color;
    COLORREF text_color;
    COLORREF footer_color;

    // [system]
    char     cpu[ARDC_LENGTH_SHORT];
    uint16_t win;
    uint16_t winnt;
    uint16_t ossp;

    // [dependencies]
    size_t           deps_count;
    ardc_dependency *deps;

    // [source.*]
    size_t       srcs_count;
    ardc_source *srcs;
} ardc_config;

extern ardc_config *
ardc_load(void);

extern void
ardc_cleanup(void);

extern const char *
ardc_get_root(void);

#endif // _ARD_CONFIG_H_
