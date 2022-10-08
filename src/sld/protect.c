
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <crg.h>
#include <dlg.h>

#include "sld_impl.h"

extern const char StrSldEnterPassword[];
extern const char StrSldEncrypted[];
extern const char StrSldKeyInvalid[];

#define _isctypestr(predicate)                                                 \
    {                                                                          \
        if (!*str)                                                             \
        {                                                                      \
            return false;                                                      \
        }                                                                      \
                                                                               \
        while (*str)                                                           \
        {                                                                      \
            if (!predicate(*str))                                              \
            {                                                                  \
                return false;                                                  \
            }                                                                  \
            str++;                                                             \
        }                                                                      \
                                                                               \
        return true;                                                           \
    }

static bool
_isdigitstr(const char *str)
{
    _isctypestr(isdigit);
}

static bool
_isxdigitstr(const char *str)
{
    _isctypestr(isxdigit);
}

bool
__sld_prompt_passcode(uint8_t               *code,
                      int                    code_len,
                      int                    base,
                      sld_passcode_validator validator,
                      void                  *context)
{
    char *buffer = (char *)alloca(code_len * 3 + 1);
    while (true)
    {
        bool (*precheck)(const char *) = NULL;
        switch (base)
        {
        case 10:
            precheck = _isdigitstr;
            break;
        case 16:
            precheck = _isxdigitstr;
            break;
        }

        int length = dlg_prompt(StrSldEnterPassword, StrSldEncrypted, buffer,
                                code_len * 2, precheck);
        if (0 == length)
        {
            return false;
        }

        if (0 == base)
        {
            memcpy(code, buffer, length);
        }
        else
        {
            uint64_t value = rstrtoull(buffer, base);
            memcpy(code, &value, code_len);
        }

        if (!validator)
        {
            return true;
        }

        if (validator(code, code_len, context))
        {
            return true;
        }

        dlg_alert(StrSldEnterPassword, StrSldKeyInvalid);
    }
}
