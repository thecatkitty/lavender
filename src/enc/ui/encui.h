#ifndef _ENCUI_H_
#define _ENCUI_H_

#include <gfx.h>

enum encui_result
{
    ENCUI_OK = 1,
    ENCUI_CANCEL = 2,
    ENCUI_ABORT = 3,
    ENCUI_RETRY = 4,
    ENCUI_IGNORE = 5,
    ENCUI_YES = 6,
    ENCUI_NO = 7,
    ENCUI_TRY_AGAIN = 10,
    ENCUI_CONTINUE = 11,
    ENCUI_INCOMPLETE = -1
};

typedef bool (*encui_validator)(const char *);

extern bool
encui_alert(const char *title, const char *message);

extern bool
encui_prompt(const char     *title,
             const char     *message,
             char           *buffer,
             int             size,
             encui_validator validator);

extern int
encui_handle(void);

#endif // _ENCUI_H_
