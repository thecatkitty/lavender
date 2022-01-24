#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include <api/dos.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <ker.h>

#define KER_PIT_INPUT_FREQ      11931816667ULL
#define KER_PIT_FREQ_DIVISOR    2048ULL
#define KER_DELAY_MS_MULTIPLIER 100ULL

#define SPKR_ENABLE 3

static volatile unsigned counter = 0xFFFF;
static isr               biosIsr;

static volatile unsigned playerTicks = 0;
static uint16_t *volatile sequence = (uint16_t *)0;
static uint16_t *sequenceEnd;

static void
PlayerStop(void);

static interrupt void far
PitIsr(void);

static void
PlayerIsr(void);

unsigned
KerGetTicksFromMs(unsigned ms)
{
    uint32_t ticks = (uint32_t)ms * KER_DELAY_MS_MULTIPLIER;
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
KerStartPlayer(void *music, uint16_t length)
{
    KerDisableInterrupts();
    sequence = (uint16_t *)music;
    sequenceEnd = sequence + length;
    playerTicks = 0;
    KerEnableInterrupts();
}

void
PlayerStop(void)
{
    KerDisableInterrupts();
    sequence = (uint16_t *)0;
    _outp(0x61, _inp(0x61) & (uint8_t)~SPKR_ENABLE);
    KerEnableInterrupts();
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
    counter = 0xFFFF;
    biosIsr = KerInstallIsr(PitIsr, INT_PIT);
    PitInitChannel(0, PIT_MODE_RATE_GEN, KER_PIT_FREQ_DIVISOR);
}

void
PitDeinitialize()
{
    PlayerStop();
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

    if (0 == (counter % 10))
    {
        PlayerIsr();
    }

    _outp(PIC1_IO_COMMAND, PIC_COMMAND_EOI);
    KerEnableInterrupts();
}

void
PlayerIsr(void)
{
    if (!sequence)
        return;

    if (sequenceEnd <= sequence)
    {
        _outp(0x61, _inp(0x61) & (uint8_t)~SPKR_ENABLE);
        sequence = (uint16_t *)0;
    }

    if (0 != playerTicks)
    {
        playerTicks--;
        return;
    }

    sequence += 2;
    playerTicks = sequence[0] - 1;

    uint16_t divisor = sequence[1];
    if (0 == divisor)
    {
        _outp(0x61, _inp(0x61) & (uint8_t)~SPKR_ENABLE);
    }
    else
    {
        PitInitChannel(2, PIT_MODE_SQUARE_WAVE_GEN, divisor);
        _outp(0x61, _inp(0x61) | SPKR_ENABLE);
    }
}
