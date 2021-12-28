#ifndef _KER_H_
#define _KER_H_

#include <stdbool.h>

#ifndef EDITING
#define far __far
#else
#define far
#endif

#define interrupt __attribute__ ((interrupt))

typedef void (interrupt *isr)(void) far;

#define KER_PIT_INPUT_FREQ              11931816667
#define KER_PIT_FREQ_DIVISOR            2048
#define KER_DELAY_MS_MULTIPLIER         100
#define KER_DELAY_MS_DIVISOR            ((10000000 * KER_DELAY_MS_MULTIPLIER) * KER_PIT_FREQ_DIVISOR / KER_PIT_INPUT_FREQ)

extern bool KerIsDosBox(void);

extern void KerSleep(
    unsigned ticks);

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
