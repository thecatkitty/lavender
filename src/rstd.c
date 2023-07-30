#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <base.h>

#define _rstrtoull(condition, digxform)                                        \
    {                                                                          \
        uint64_t num = 0, prev = 0;                                            \
        while (condition(*str))                                                \
        {                                                                      \
            prev = num;                                                        \
            digxform(*str, num);                                               \
            str++;                                                             \
                                                                               \
            if (num < prev)                                                    \
            {                                                                  \
                errno = ERANGE;                                                \
                return ULLONG_MAX;                                             \
            }                                                                  \
        }                                                                      \
        return num;                                                            \
    }

#define _digxform_10(c, num)                                                   \
    {                                                                          \
        num *= 10;                                                             \
        num += c - '0';                                                        \
    }

#define _digxform_16(c, num)                                                   \
    {                                                                          \
        num <<= 4;                                                             \
        num += isdigit(c) ? (c - '0') : (tolower(c) - 'a' + 10);               \
    }

uint64_t
rstrtoull(const char *restrict str, int base)
{
    while (isspace(*str))
    {
        ++str;
    }

    switch (base)
    {
    case 10:
        _rstrtoull(isdigit, _digxform_10);

    case 16:
        _rstrtoull(isxdigit, _digxform_16);

    default:
        errno = EINVAL;
        return ULLONG_MAX;
    }
}

#if !__MISC_VISIBLE
char *
itoa(int value, char *str, int base)
{
    if (10 == base)
    {
        sprintf(str, "%d", value);
    }
    else if (16 == base)
    {
        sprintf(str, "%x", value);
    }
    else
    {
        strcpy(str, "?base?");
    }

    return str;
}
#endif
