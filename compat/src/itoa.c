#include <stdio.h>
#include <string.h>

#include <compat/stdlib.h>

#if defined(CUSTOM_ITOA)
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
#else
typedef int this_unit_is_intentionally_left_blank;
#endif
