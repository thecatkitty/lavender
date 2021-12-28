#ifndef _KER_H_
#define _KER_H_

#ifndef EDITING
#define far __far
#else
#define far
#endif

typedef void far *isr;

extern isr KerInstallIsr(
    isr      routine,
    unsigned number);

extern void KerUninstallIsr(
    isr      previous,
    unsigned number
    );

void KerDisableInterrupts()
{
    asm ("cli");
}

void KerEnableInterrupts()
{
    asm ("sti");
}

#endif // _KER_H_
