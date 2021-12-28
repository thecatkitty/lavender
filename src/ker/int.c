#include <ker.h>

static isr far * const vectors = (isr far *)0x00000000;

isr KerInstallIsr(
    isr      routine,
    unsigned number)
{
    KerDisableInterrupts();
    isr previous = vectors[number];
    vectors[number] = routine;
    KerEnableInterrupts();
    return previous;
}

void KerUninstallIsr(
    isr      previous,
    unsigned number
    )
{
    KerDisableInterrupts();
    vectors[number] = previous;
    KerEnableInterrupts();
}
