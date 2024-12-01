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

enum
{
    ENCUIFT_SEPARATOR,
    ENCUIFT_LABEL,
    ENCUIFT_TEXTBOX,
    ENCUIFT_CHECKBOX,
};

#define ENCUIFF_STATIC  (0 << 0)
#define ENCUIFF_DYNAMIC (1 << 0)

#define ENCUIFF_LEFT   (0 << 1)
#define ENCUIFF_CENTER (1 << 1)
#define ENCUIFF_RIGHT  (2 << 1)
#define ENCUIFF_ALIGN  (3 << 1)

#define ENCUIFF_UNCHECKED (0 << 3)
#define ENCUIFF_CHECKED   (1 << 3)

typedef int(encui_page_proc)(int msg, void *param, void *data);

typedef struct
{
    char       *buffer;
    size_t      capacity;
    size_t      length;
    const char *alert;
} encui_prompt_page;

typedef struct
{
    int      type;
    int      flags;
    intptr_t data;
} encui_field;

typedef struct
{
    int          length;
    encui_field *fields;
} encui_complex_page;

typedef struct
{
    int              title;
    int              message;
    encui_page_proc *proc;
    void            *data;

    union {
        encui_prompt_page  prompt;
        encui_complex_page cpx;
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

extern encui_field *
encui_find_checkbox(encui_page *page);

extern encui_prompt_page *
encui_find_prompt(encui_page *page);

#endif // _ENCUI_H_
