#include <string.h>

#include <api/dos.h>
#include <ker.h>

extern char __edata[], __sbss[], __ebss[];

extern int
Main(ZIP_CDIR_END_HEADER *zip);

extern void
PitInitialize(void);

extern void
PitDeinitialize(void);

DOS_PSP *KerPsp;

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
    memset(__sbss, 0, __ebss - __sbss);
    KerPsp = (DOS_PSP *)0;

    ZIP_CDIR_END_HEADER *zip;
    if (0 > KerLocateArchive(__edata, __sbss, &zip))
    {
        KerTerminate();
    }

    PitInitialize();

    int exitCode = Main(zip);

    PitDeinitialize();

    DosExit(exitCode);
}
