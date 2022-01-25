#include <api/dos.h>
#include <ker.h>

bool
KerIsDosBox(void)
{
    const char far *str = (const char far *)0xF000E061;
    for (const char *i = "DOSBox"; *i; i++, str++)
    {
        if (*i != *str)
            return false;
    }
    return true;
}

bool
KerIsDosMajor(uint8_t major)
{
    uint8_t dosMajor = DosGetVersion() & 0xFF;
    return (1 == major) ? (0 == dosMajor) : (major == dosMajor);
}
