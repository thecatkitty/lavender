#include <conio.h>
#include <i86.h>

#ifdef CONFIG_ANDREA
#include <andrea.h>
#endif

#include <arch/dos.h>

#include "hw.h"

volatile uint32_t dos_counter;

uint32_t ddcall
pal_get_counter(void)
{
    uint16_t count = 0;

    _disable();
    outp(PIT_IO_COMMAND, 0x00);
    count = inp(PIT_DATA(0));
    count |= inp(PIT_DATA(0)) << 8;
    _enable();

    return (dos_counter << PIT_FREQ_POWER) |
           (count & ((1 << PIT_FREQ_POWER) - 1));
}

uint32_t ddcall
pal_get_ticks(unsigned ms)
{
    uint64_t ticks = (uint64_t)ms * (PIT_INPUT_FREQ_F10 / 1000ULL);
    ticks >>= 10;
    return (UINT32_MAX < ticks) ? UINT32_MAX : ticks;
}

void ddcall
pal_sleep(unsigned ms)
{
    uint32_t until = pal_get_counter() + pal_get_ticks((uint32_t)ms);
    while (pal_get_counter() < until)
    {
        asm("hlt");
    }
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(pal_get_counter);
ANDREA_EXPORT(pal_get_ticks);
ANDREA_EXPORT(pal_sleep);
#endif
