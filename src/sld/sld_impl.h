#ifndef _SLD_IMPL_H_
#define _SLD_IMPL_H_

#include <crg.h>
#include <sld.h>

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

extern bool
__sld_execute_bitmap(sld_entry *sld);

extern bool
__sld_prompt_passcode(uint8_t               *code,
                      int                    code_len,
                      int                    base,
                      sld_passcode_validator validator,
                      void                  *context);

#endif // _SLD_IMPL_H_
