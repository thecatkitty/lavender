#include <fmt/spk.h>
#include <pal.h>
#include <snd.h>

static volatile unsigned _ticks = 0;
static spk_note3 *volatile _sequence = (spk_note3 *)0;

static uint32_t _tick_rate;
static uint32_t _next_tick;

void
snd_handle(void)
{
    if (!_sequence)
        return;

    uint32_t now = pal_get_counter();
    if (_next_tick > now)
    {
        return;
    }

    _next_tick = now + _tick_rate;
    if (0 != _ticks)
    {
        _ticks--;
        return;
    }

    if (SPK_NOTE_DURATION_STOP == _sequence->duration)
    {
        snd_silence();
        return;
    }

    _ticks = _sequence->duration - 1;
    if (0 == _sequence->divisor)
    {
        snd_silence();
    }
    else
    {
        snd_beep(_sequence->divisor);
    }

    _sequence++;
}

void
snd_play(void *music, uint16_t length)
{
    spk_header *header = (spk_header *)music;
    _tick_rate = pal_get_ticks(1000 / header->ticks_per_second);
    _sequence = (spk_note3 *)((uint8_t *)music + sizeof(spk_header));
    _ticks = 0;
}
