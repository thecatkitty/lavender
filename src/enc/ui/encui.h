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
    char  *buffer;
    size_t capacity;
    size_t length;
} encui_prompt_page;

typedef struct
{
    int              title;
    int              message;
    encui_page_proc *proc;
    void            *data;

    union {
        encui_prompt_page prompt;
    };
} encui_page;

extern bool
encui_enter(encui_page *pages, int count);

extern bool
encui_exit(void);

extern int
encui_get_page(void);

extern bool
encui_set_page(int id);

extern int
encui_handle(void);

#endif // _ENCUI_H_
