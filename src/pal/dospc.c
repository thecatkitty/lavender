#include <stdlib.h>
#include <string.h>

#include <api/dos.h>
#include <ker.h>
#include <pal.h>

extern char __edata[], __sbss[], __ebss[];

extern void
PitInitialize(void);

extern void
PitDeinitialize(void);

#ifdef STACK_PROFILING
static uint64_t      *_stack_start;
static uint64_t      *_stack_end = (uint64_t *)0xFFE8;
static const uint64_t STACK_FILL_PATTERN = 0x0123456789ABCDEFULL;
#endif // STACK_PROFILING

void
pal_initialize(ZIP_CDIR_END_HEADER **zip)
{
    memset(__sbss, 0, __ebss - __sbss);

#ifdef STACK_PROFILING
    _stack_start = (uint64_t *)((unsigned)__ebss / 8 * 8) + 1;
    for (uint64_t *ptr = _stack_start; ptr < _stack_end; ptr++)
        *ptr = STACK_FILL_PATTERN;
#endif // STACK_PROFILING

    int status;
    if (0 > (status = KerLocateArchive(__edata, __sbss, zip)))
    {
        KerTerminate(-status);
    }

    PitInitialize();
}

void
pal_cleanup(void)
{
    PitDeinitialize();

#ifdef STACK_PROFILING
    uint64_t *untouched;
    for (untouched = _stack_start; untouched < _stack_end; untouched++)
    {
        if (STACK_FILL_PATTERN != *untouched)
            break;
    }

    int stack_size = (int)(0x10000UL - (uint16_t)untouched);

    DosPutS("Stack usage: $");

    char buffer[6];
    itoa(stack_size, buffer, 10);
    for (int i = 0; buffer[i]; i++)
        DosPutC(buffer[i]);

    DosPutS("\r\n$");
#endif // STACK_PROFILING
}
