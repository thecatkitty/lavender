#include <base.h>

static unsigned long
_divmodul(unsigned long a, unsigned long b, unsigned long *c)
{
    unsigned long quotient = 0;

    for (int i = 31; i >= 0; --i)
    {
        if ((b << i) <= a)
        {
            a -= (b << i);
            quotient |= (1LL << i);
        }
    }

    if (NULL != c)
    {
        *c = a;
    }

    return quotient;
}

unsigned long
__udivdi3(unsigned long a, unsigned long b)
{
    return _divmodul(a, b, NULL);
}

unsigned long
__umoddi3(unsigned long a, unsigned long b)
{
    unsigned long remainder = 0;
    _divmodul(a, b, &remainder);
    return remainder;
}
