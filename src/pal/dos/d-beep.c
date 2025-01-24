#include <conio.h>

#include <arch/dos.h>

#ifdef CONFIG_ANDREA
#include <andrea.h>
#endif

#include "hw.h"

void ddcall
dos_beep(uint16_t divisor)
{
    pit_init_channel(2, PIT_MODE_SQUARE_WAVE_GEN, divisor);
    _outp(0x61, _inp(0x61) | SPKR_ENABLE);
}

void ddcall
dos_silence(void)
{
    _outp(0x61, _inp(0x61) & ~SPKR_ENABLE);
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(dos_beep);
ANDREA_EXPORT(dos_silence);
#endif
