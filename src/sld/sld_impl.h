#ifndef _SLD_IMPL_H_
#define _SLD_IMPL_H_

#include <crg.h>
#include <sld.h>

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

extern void
__sld_errmsgcpy(void *sld, const char *msg);

extern void
__sld_errmsgcat(void *sld, const char *msg);

extern uint16_t       __sld_accumulator;
extern gfx_dimensions __sld_screen;

extern const char IDS_LOADERROR[];
extern const char IDS_EXECERROR[];
extern const char IDS_NOEXECCTX[];
extern const char IDS_INVALIDDELAY[];
extern const char IDS_UNKNOWNTYPE[];
extern const char IDS_INVALIDVPOS[];
extern const char IDS_INVALIDHPOS[];
extern const char IDS_LONGNAME[];
extern const char IDS_LONGCONTENT[];
extern const char IDS_NOLABEL[];
extern const char IDS_INVALIDCMPVAL[];
extern const char IDS_NOASSET[];
extern const char IDS_BADENCODING[];
extern const char IDS_UNSUPPORTED[];

#endif // _SLD_IMPL_H_
