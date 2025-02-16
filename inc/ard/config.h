#ifndef _ARD_CONFIG_H_
#define _ARD_CONFIG_H_

#include <minwindef.h>
#include <stdbool.h>
#include <stdint.h>

#define ARDC_LENGTH_LONG  MAX_PATH
#define ARDC_LENGTH_MID   80
#define ARDC_LENGTH_SHORT 20

typedef struct
{
    char     name[ARDC_LENGTH_SHORT];
    uint16_t version;
} ardc_dependency;

typedef struct
{
    // [lard]
    char name[ARDC_LENGTH_MID];
    char run[ARDC_LENGTH_LONG];
    char rundos[ARDC_LENGTH_LONG];

    // [system]
    char     cpu[ARDC_LENGTH_SHORT];
    uint16_t win;
    uint16_t winnt;
    uint16_t ossp;

    // [dependencies]
    size_t           deps_count;
    ardc_dependency *deps;
} ardc_config;

extern ardc_config *
ardc_load(void);

extern void
ardc_cleanup(void);

#endif // _ARD_CONFIG_H_
