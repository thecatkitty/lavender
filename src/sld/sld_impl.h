#ifndef _SLD_IMPL_H_
#define _SLD_IMPL_H_

#include <crg.h>

typedef bool (*sld_passcode_validator)(const uint8_t *code,
                                       int            length,
                                       void          *context);
extern bool
__sld_prompt_passcode(uint8_t               *code,
                      int                    code_len,
                      int                    base,
                      sld_passcode_validator validator,
                      void                  *context);

#endif // _SLD_IMPL_H_
