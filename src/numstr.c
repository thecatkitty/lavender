#include <ctype.h>
#include <errno.h>

#include <numstr.h>

#define RSTRTOULL(str, num, condition, digxform)                               \
    {                                                                          \
        uint64_t prev = 0;                                                     \
        while (condition(*str))                                                \
        {                                                                      \
            prev = num;                                                        \
            digxform(*str, num);                                               \
            str++;                                                             \
                                                                               \
            if (num < prev)                                                    \
            {                                                                  \
                errno = ERANGE;                                                \
                num = UINT64_MAX;                                              \
                break;                                                         \
            }                                                                  \
        }                                                                      \
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
rstrtoull(const char *str, int base)
{
    uint64_t num = 0;

    while (isspace(*str))
    {
        ++str;
    }

    switch (base)
    {
    case 10:
        RSTRTOULL(str, num, isdigit, _digxform_10);
        return num;

    case 16:
        RSTRTOULL(str, num, isxdigit, _digxform_16);
        return num;

    default:
        errno = EINVAL;
        return UINT64_MAX;
    }
}

#define ISCTYPESTR(predicate)                                                  \
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

bool
isdigstr(const char *str)
{
    ISCTYPESTR(isdigit);
}

bool
isxdigstr(const char *str)
{
    ISCTYPESTR(isxdigit);
}

uint8_t
xtob(const char *str)
{
    uint8_t ret;

    if (!isxdigit(str[0]) || !isxdigit(str[1]))
    {
        return 0;
    }

    ret = isdigit(str[1]) ? (str[1] - '0') : (toupper(str[1]) - 'A' + 10);
    ret |= (isdigit(str[0]) ? (str[0] - '0') : (toupper(str[0]) - 'A' + 10))
           << 4;

    return ret;
}
