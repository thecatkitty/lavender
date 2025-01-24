#include <conio.h>
#include <dos.h>

#include <arch/dos.h>

#include "hw.h"

extern void
dos_pit_isr(void);

extern uint16_t   dos_data_segment;
volatile uint32_t dos_counter;
dos_isr           dos_pit_original_isr;

void
pit_initialize(void)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    _disable();
    dos_pit_original_isr = _dos_getvect(INT_PIT);
    _dos_setvect(INT_PIT, MK_FP(__libi86_get_cs(), dos_pit_isr));

    asm volatile("movw %%ds, %%cs:%0" : "=rm"(dos_data_segment));
    dos_counter = 0;
    pit_init_channel(0, PIT_MODE_RATE_GEN, PIT_FREQ_DIVISOR);
    _enable();
#pragma GCC diagnostic pop
}

void
pit_cleanup(void)
{
    _dos_setvect(INT_PIT, dos_pit_original_isr);

    _disable();
    pit_init_channel(0, PIT_MODE_RATE_GEN, 0);
    _enable();
}

void
pit_init_channel(unsigned channel, unsigned mode, unsigned divisor)
{
    if (channel > 2)
        return;
    mode &= PIT_MODE_MASK;

    _outp(PIT_IO_COMMAND, (mode << PIT_MODE) | (channel << PIT_CHANNEL) |
                              (1 << PIT_BYTE_HI) | (1 << PIT_BYTE_LO));
    _outp(PIT_IO + channel, divisor & 0xFF);
    _outp(PIT_IO + channel, divisor >> 8);
}
