#include <stdlib.h>
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

static const uint64_t STACK_FILL_PATTERN = 0x0123456789ABCDEFULL;

void
KerEntry(void)
{
    int status;

    memset(__sbss, 0, __ebss - __sbss);

#ifdef STACK_PROFILING
    uint64_t *stackStart = (uint64_t *)((unsigned)__ebss / 8 * 8) + 1;
    for (uint64_t *ptr = stackStart; ptr < (uint64_t *)0xFFF8; ptr++)
        *ptr = STACK_FILL_PATTERN;
#endif // STACK_PROFILING

    KerPsp = (DOS_PSP *)0;
    ZIP_CDIR_END_HEADER *zip;
    if (0 > (status = KerLocateArchive(__edata, __sbss, &zip)))
    {
        KerTerminate(-status);
    }

    PitInitialize();

    status = Main(zip);

    PitDeinitialize();

#ifdef STACK_PROFILING
    uint64_t *untouched;
    for (untouched = stackStart; untouched < (uint64_t *)0xFFF8; untouched++)
    {
        if (STACK_FILL_PATTERN != *untouched)
            break;
    }

    int stackSize = (int)(0x10000UL - (uint16_t)untouched);

    DosPutS("Stack usage: $");

    char buffer[6];
    itoa(stackSize, buffer, 10);
    for (int i = 0; buffer[i]; i++)
        DosPutC(buffer[i]);

    DosPutS("\r\n$");
#endif // STACK_PROFILING

    if (0 > status)
    {
        KerTerminate(-status);
    }

    DosExit(status);
}
