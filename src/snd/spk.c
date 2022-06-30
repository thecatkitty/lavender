#include <fmt/spk.h>
#include <pal.h>
#include <pal/dospc.h>
#include <snd.h>

static volatile unsigned _ticks = 0;
static spk_note3 *volatile _sequence = (spk_note3 *)0;

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

    if (SPK_NOTE_DURATION_STOP == _sequence->duration)
    {
        pal_unregister_timer_callback(_timer);
        dospc_silence();
        return;
    }

    _ticks = _sequence->duration - 1;
    if (0 == _sequence->divisor)
    {
        dospc_silence();
    }
    else
    {
        dospc_beep(_sequence->divisor);
    }

    _sequence++;
}

void
snd_play(void *music, uint16_t length)
{
    _sequence = (spk_note3 *)((uint8_t *)music + sizeof(spk_header));
    _ticks = 0;

    _timer = pal_register_timer_callback(_callback, NULL);
}
