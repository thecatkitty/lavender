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
    // [lard]
    char name[ARDC_LENGTH_MID];
    char run[ARDC_LENGTH_LONG];

    // [system]
    char     cpu[ARDC_LENGTH_SHORT];
    uint16_t win;
    uint16_t winnt;
} ardc_config;

extern ardc_config *
ardc_load(void);

#endif // _ARD_CONFIG_H_
