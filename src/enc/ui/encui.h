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
    ENCUIM_INIT,
    ENCUIM_ENTERED,
    ENCUIM_NOTIFY,
    ENCUIM_CHECK,
    ENCUIM_NEXT,
};

enum
{
    ENCUIFT_SEPARATOR,
    ENCUIFT_LABEL,
    ENCUIFT_TEXTBOX,
    ENCUIFT_CHECKBOX,
    ENCUIFT_OPTION,
    ENCUIFT_BITMAP,
};

#define ENCUIFF_STATIC  (0 << 0)
#define ENCUIFF_DYNAMIC (1 << 0)

#define ENCUIFF_LEFT   (0 << 1)
#define ENCUIFF_CENTER (1 << 1)
#define ENCUIFF_RIGHT  (2 << 1)
#define ENCUIFF_ALIGN  (3 << 1)

#define ENCUIFF_BODY     (0 << 3)
#define ENCUIFF_FOOTER   (1 << 3)
#define ENCUIFF_POSITION (1 << 3)

#define ENCUIFF_UNCHECKED (0 << 3)
#define ENCUIFF_CHECKED   (1 << 3)

typedef int(encui_page_proc)(int msg, void *param, void *data);

typedef struct
{
    char       *buffer;
    size_t      capacity;
    size_t      length;
    const char *alert;
} encui_textbox_data;

typedef struct
{
    int      type;
    int      flags;
    intptr_t data;
} encui_field;

typedef struct
{
    int              title;
    encui_page_proc *proc;
    void            *data;
    int              length;
    encui_field     *fields;
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

extern encui_textbox_data *
encui_find_textbox(encui_page *page);

extern int
encui_check_page(const encui_page *page, void *param);

#ifdef _WIN32
extern bool
encui_refresh_field(encui_page *page, int id);

extern bool
encui_request_notify(int cookie);
#endif

#endif // _ENCUI_H_
