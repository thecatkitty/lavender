#include <conio.h>
#include <dos.h>
#include <stdint.h>

#include <api/dos.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <fmt/spk.h>
#include <ker.h>
#include <pal.h>

static volatile unsigned s_PlayerTicks = 0;
static SPK_NOTE3 *volatile s_Sequence = (SPK_NOTE3 *)0;

static void
PlayerStop(void);

static void
PlayerIsr(void);

void
KerStartPlayer(void *music, uint16_t length)
{
    _disable();
    s_Sequence = (SPK_NOTE3 *)((uint8_t *)music + sizeof(SPK_HEADER));
    s_PlayerTicks = 0;
    _enable();
}

void
PlayerStop(void)
{
    _disable();
    s_Sequence = (SPK_NOTE3 *)0;
    nosound();
    _enable();
}

void
PlayerIsr()
{
    if (!s_Sequence)
        return;

    if (0 != s_PlayerTicks)
    {
        s_PlayerTicks--;
        return;
    }

    if (SPK_NOTE_DURATION_STOP == s_Sequence->Duration)
    {
        PlayerStop();
        return;
    }

    s_PlayerTicks = s_Sequence->Duration - 1;
    if (0 == s_Sequence->Divisor)
    {
        nosound();
    }
    else
    {
        pal_beep(s_Sequence->Divisor);
    }

    s_Sequence++;
}
