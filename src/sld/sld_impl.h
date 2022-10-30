#ifndef _SLD_IMPL_H_
#define _SLD_IMPL_H_

#include <string.h>

#include <crg.h>
#include <sld.h>

#include "../resource.h"

#define LINE_WIDTH 80

#define __sld_try_load(stage, str, out)                                        \
    {                                                                          \
        int length;                                                            \
        if (0 > (length = stage(str, out)))                                    \
        {                                                                      \
            return length;                                                     \
        };                                                                     \
        str += length;                                                         \
    }

typedef bool (*sld_passcode_validator)(const uint8_t *code,
                                       int            length,
                                       void          *context);

extern int
__sld_loadu(const char *str, uint16_t *out);

extern int
__sld_load_position(const char *str, sld_entry *out);

extern int
__sld_load_content(const char *str, sld_entry *out);

extern int
__sld_load_bitmap(const char *str, sld_entry *out);

extern int
__sld_load_conditional(const char *str, sld_entry *out);

extern int
__sld_load_text(const char *str, sld_entry *out);

extern int
__sld_load_shape(const char *str, sld_entry *out);

extern int
__sld_load_script_call(const char *str, sld_entry *out);

extern bool
__sld_execute_bitmap(sld_entry *sld);

extern int
__sld_execute_text(sld_entry *sld);

extern int
__sld_execute_play(sld_entry *sld);

extern int
__sld_execute_rectangle(sld_entry *sld);

extern int
__sld_execute_script_call(sld_entry *sld);

static inline void
__sld_errmsgcpy(void *sld, unsigned int msg)
{
    pal_load_string(msg, (char *)sld, sizeof(sld_entry));
}

static inline void
__sld_errmsgcat(void *sld, const char *msg)
{
    strncat((char *)sld, msg, sizeof(sld_entry) - strlen((char *)sld));
}

extern uint16_t       __sld_accumulator;
extern sld_context   *__sld_ctx;
extern gfx_dimensions __sld_screen;

#endif // _SLD_IMPL_H_
