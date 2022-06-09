#include <stdlib.h>
#include <string.h>

#include <api/dos.h>
#include <ker.h>
#include <pal.h>

extern int
Main(ZIP_CDIR_END_HEADER *zip);

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
    ZIP_CDIR_END_HEADER *zip;
    pal_initialize(&zip);

    int status = Main(zip);

    pal_cleanup();

    if (0 > status)
    {
        KerTerminate(-status);
    }

    DosExit(status);
}
