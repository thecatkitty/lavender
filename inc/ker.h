#ifndef _KER_H_
#define _KER_H_

#ifndef EDITING
#define far __far
#else
#define far
#endif

#define interrupt __attribute__ ((interrupt))

typedef void (interrupt *isr)(void) far;

extern isr KerInstallIsr(
    isr      routine,
    unsigned number);

extern void KerUninstallIsr(
    isr      previous,
    unsigned number
    );

inline void KerDisableInterrupts()
{
    asm ("cli");
}

inline void KerEnableInterrupts()
{
    asm ("sti");
}

#endif // _KER_H_
