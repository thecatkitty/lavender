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
#ifdef TODO__PLAYER
    _disable();
    s_Sequence = (SPK_NOTE3 *)((uint8_t *)music + sizeof(SPK_HEADER));
    s_PlayerTicks = 0;
    _enable();
#endif
}

void
PlayerStop(void)
{
#ifdef TODO__PLAYER
    _disable();
    s_Sequence = (SPK_NOTE3 *)0;
    nosound();
    _enable();
#endif
}

void
PlayerIsr()
{
#ifdef TODO__PLAYER
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
#endif
}
