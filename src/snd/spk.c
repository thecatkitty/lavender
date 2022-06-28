#include <dos.h>

#include <fmt/spk.h>
#include <snd.h>
#include <pal.h>

static volatile unsigned _ticks = 0;
static SPK_NOTE3 *volatile _sequence = (SPK_NOTE3 *)0;

static htimer _timer;

static void
_callback(void *context)
{
    if (!_sequence)
        return;

    if (0 != _ticks)
    {
        _ticks--;
        return;
    }

    if (SPK_NOTE_DURATION_STOP == _sequence->Duration)
    {
        pal_unregister_timer_callback(_timer);
        nosound();
        return;
    }

    _ticks = _sequence->Duration - 1;
    if (0 == _sequence->Divisor)
    {
        nosound();
    }
    else
    {
        pal_beep(_sequence->Divisor);
    }

    _sequence++;
}

void
snd_play(void *music, uint16_t length)
{
    _sequence = (SPK_NOTE3 *)((uint8_t *)music + sizeof(SPK_HEADER));
    _ticks = 0;

    _timer = pal_register_timer_callback(_callback, NULL);
}
