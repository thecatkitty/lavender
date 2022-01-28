#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include <api/dos.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <fmt/spk.h>
#include <ker.h>

#define KER_PIT_INPUT_FREQ      11931816667ULL
#define KER_PIT_FREQ_DIVISOR    2048ULL
#define KER_DELAY_MS_MULTIPLIER 100ULL

#define SPKR_ENABLE 3

static volatile uint32_t counter;
static isr               biosIsr;

static volatile unsigned playerTicks = 0;
static SPK_NOTE3 *volatile sequence = (SPK_NOTE3 *)0;

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
    uint32_t until = counter + ticks;
    while (counter != until)
    {
        asm("hlt");
    }
}

void
KerStartPlayer(void *music, uint16_t length)
{
    _disable();
    sequence = (SPK_NOTE3 *)((uint8_t *)music + sizeof(SPK_HEADER));
    playerTicks = 0;
    _enable();
}

void
PlayerStop(void)
{
    _disable();
    sequence = (SPK_NOTE3 *)0;
    nosound();
    _enable();
}

void
PitInitChannel(unsigned channel, unsigned mode, unsigned divisor)
{
    if (channel > 2)
        return;
    mode &= PIT_MODE_MASK;

    _disable();
    _outp(PIT_IO_COMMAND, (mode << PIT_MODE) | (channel << PIT_CHANNEL) |
                              (1 << PIT_BYTE_HI) | (1 << PIT_BYTE_LO));
    _outp(PIT_IO + channel, divisor & 0xFF);
    _outp(PIT_IO + channel, divisor >> 8);
    _enable();
}

void
PitInitialize()
{
    _disable();
    biosIsr = _dos_getvect(INT_PIT);
    _dos_setvect(INT_PIT, PitIsr);
    _enable();

    counter = 0;
    PitInitChannel(0, PIT_MODE_RATE_GEN, KER_PIT_FREQ_DIVISOR);
}

void
PitDeinitialize()
{
    PlayerStop();
    _dos_setvect(INT_PIT, biosIsr);
    PitInitChannel(0, PIT_MODE_RATE_GEN, 0);
}

interrupt void far
PitIsr(void)
{
    _disable();
    counter++;

    if (0 == (counter % 10))
    {
        PlayerIsr();
    }

    if (0 == (counter % 32))
    {
        biosIsr();
    }
    else
    {
        _outp(PIC1_IO_COMMAND, PIC_COMMAND_EOI);
    }

    _enable();
}

void
PlayerIsr(void)
{
    if (!sequence)
        return;

    if (0 != playerTicks)
    {
        playerTicks--;
        return;
    }

    if (SPK_NOTE_DURATION_STOP == sequence->Duration)
    {
        PlayerStop();
        return;
    }

    playerTicks = sequence->Duration - 1;
    if (0 == sequence->Divisor)
    {
        nosound();
    }
    else
    {
        PitInitChannel(2, PIT_MODE_SQUARE_WAVE_GEN, sequence->Divisor);
        _outp(0x61, _inp(0x61) | SPKR_ENABLE);
    }

    sequence++;
}
