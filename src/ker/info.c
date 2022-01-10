#include <stdint.h>

#include <ker.h>

bool KerIsDosBox(void)
{
    const char far * str = (const char far *)0xF000E061;
    for (const char* i = "DOSBox"; *i; i++, str++)
    {
        if (*i != *str) return false;
    }
    return true;
}
