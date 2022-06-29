#include <ctype.h>
#include <errno.h>

#include <base.h>

#define _rstrtoull(condition, digxform)                                        \
    {                                                                          \
        long long num = 0, prev = 0;                                           \
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

long long
rstrtoull(const char *restrict str, int base)
{
    while (isspace(*str++))
        ;

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
