#include <dos.h>
#include <stdint.h>

#include <ker.h>

isr
KerInstallIsr(isr routine, unsigned number)
{
    _disable();
    isr previous = _dos_getvect(number);
    _dos_setvect(number, routine);
    _enable();
    return previous;
}

void
KerUninstallIsr(isr previous, unsigned number)
{
    _disable();
    _dos_setvect(number, previous);
    _enable();
}
