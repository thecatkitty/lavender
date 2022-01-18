#include <api/dos.h>
#include <ker.h>

extern int
Main(void);

extern void
PitInitialize(void);

extern void
PitDeinitialize(void);

void
_start(void) __attribute__((section(".startupA.0")));

void
KerEntry(void) __attribute__((section(".startupB")));

void
_start(void)
{
    asm("jmp KerEntry");
}

void
KerEntry(void)
{
    PitInitialize();

    int exitCode = Main();

    PitDeinitialize();

    DosExit(exitCode);
}
