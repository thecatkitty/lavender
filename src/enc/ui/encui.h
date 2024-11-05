#ifndef _ENCUI_H_
#define _ENCUI_H_

#include <gfx.h>

#include "../enc_impl.h"

enum encui_result
{
    ENCUI_OK = 1 << 0,
    ENCUI_CANCEL = 1 << 1,
    ENCUI_INCOMPLETE = -1
};

enum
{
    ENCUIM_CHECK,
    ENCUIM_NEXT,
};

typedef int(encui_page_proc)(int msg, void *param, void *data);

typedef struct
{
    int              title;
    int              message;
    char            *buffer;
    size_t           capacity;
    encui_page_proc *proc;
    void            *data;
} encui_page;

extern bool
encui_enter(void);

extern bool
encui_exit(void);

extern bool
encui_prompt(encui_page *page);

extern int
encui_handle(void);

#endif // _ENCUI_H_
