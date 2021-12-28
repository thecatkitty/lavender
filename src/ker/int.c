#include <ker.h>

#include <dos.h>

isr KerInstallIsr(
    isr      routine,
    unsigned number)
{
    KerDisableInterrupts();
    isr previous = _dos_getvect(number);
    _dos_setvect(number, routine);
    KerEnableInterrupts();
    return previous;
}

void KerUninstallIsr(
    isr      previous,
    unsigned number
    )
{
    KerDisableInterrupts();
    _dos_setvect(number, previous);
    KerEnableInterrupts();
}
