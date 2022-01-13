#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include <dev/pic.h>
#include <dev/pit.h>
#include <ker.h>

#define KER_PIT_INPUT_FREQ      11931816667ULL
#define KER_PIT_FREQ_DIVISOR    2048ULL
#define KER_DELAY_MS_MULTIPLIER 100ULL

static volatile unsigned counter = 0xFFFF;
static isr               biosIsr;

static interrupt void far
PitIsr(void);

unsigned
KerGetTicksFromMs(unsigned ms)
{
    unsigned ticks = ms * KER_DELAY_MS_MULTIPLIER;
    ticks /= (10000000UL * KER_DELAY_MS_MULTIPLIER) * KER_PIT_FREQ_DIVISOR /
             KER_PIT_INPUT_FREQ;
    return ticks;
}

void
KerSleep(unsigned ticks)
{
    unsigned last = counter;
    while (0 != ticks)
    {
        asm("hlt");
        if (counter == last)
            continue;
        ticks--;
    }
}

void
PitInitChannel(unsigned channel, unsigned mode, unsigned divisor)
{
    if (channel > 2)
        return;
    mode &= PIT_MODE_MASK;

    KerDisableInterrupts();
    _outp(PIT_IO_COMMAND, (mode << PIT_MODE) | (channel << PIT_CHANNEL) |
                              (1 << PIT_BYTE_HI) | (1 << PIT_BYTE_LO));
    _outp(PIT_IO + channel, divisor & 0xFF);
    _outp(PIT_IO + channel, divisor >> 8);
    KerEnableInterrupts();
}

void
PitInitialize()
{
    biosIsr = KerInstallIsr(PitIsr, INT_PIT);
    PitInitChannel(0, PIT_MODE_RATE_GEN, KER_PIT_FREQ_DIVISOR);
}

void
PitDeinitialize()
{
    KerUninstallIsr(biosIsr, INT_PIT);
    PitInitChannel(0, PIT_MODE_RATE_GEN, 0);
}

interrupt void far
PitIsr(void)
{
    KerDisableInterrupts();
    counter++;

    if (0 == (counter % 32))
    {
        biosIsr();
    }

    _outp(PIC1_IO_COMMAND, PIC_COMMAND_EOI);
    KerEnableInterrupts();
}
