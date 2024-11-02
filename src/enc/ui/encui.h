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

typedef bool (*encui_validator)(const char *);

extern bool
encui_enter(void);

extern bool
encui_exit(void);

extern bool
encui_prompt(const char     *title,
             const char     *message,
             char           *buffer,
             int             size,
             encui_validator validator,
             enc_context    *enc);

extern int
encui_handle(void);

#endif // _ENCUI_H_
