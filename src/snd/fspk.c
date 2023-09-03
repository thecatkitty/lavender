#include <fmt/spk.h>
#include <pal.h>
#include <snd.h>

#define NOTE_CONST_U32F8 52700

// Binary logarithm of range <1, 2), as 8-bit fractional part.
static const uint8_t LB_LOOKUP[] = {
    0,   // lb(1.0000)    0.000
    22,  // lb(1.0625)    0.087
    44,  // lb(1.1250)    0.170
    63,  // lb(1.1875)    0.248
    82,  // lb(1.2500)    0.322
    100, // lb(1.3125)    0.392
    118, // lb(1.3750)    0.459
    134, // lb(1.4375)    0.524
    150, // lb(1.5000)    0.585
    165, // lb(1.5625)    0.644
    179, // lb(1.6250)    0.700
    193, // lb(1.6875)    0.755
    207, // lb(1.7500)    0.807
    220, // lb(1.8125)    0.858
    232, // lb(1.8750)    0.907
    244  // lb(1.9375)    0.954
};

static unsigned   _ticks = 0;
static spk_note3 *_sequence = (spk_note3 *)0;

static uint32_t _tick_rate;
static uint32_t _next_tick;
static uint8_t  _last_note;

// Binary logarithm approximation (8-bit fixed point) of given integer
// lb(x) = lb(2^n * x * 2^(-n)), n natural
//       = lb(2^n) + lb(x * 2^(-n))
//       = n + lb(x / 2^n)
//       = n + lb(y), y in <1, 2)
static uint32_t
_lb(uint32_t x)
{
    uint8_t  n = 0;
    uint32_t temp_x = x;
    while (temp_x >>= 1)
    {
        n++;
    }

    uint32_t y = (x << 8) >> n; // 8-bit fixed point
    uint8_t  index = (y - 256) >> 4, remainder = (y - 256) & 0xF;

    uint16_t left = LB_LOOKUP[index] * (15 - remainder);
    uint16_t right =
        (index < 15) ? (LB_LOOKUP[index + 1] * remainder) : (remainder << 8);
    uint16_t frac_part = (left + right) / 15;

    return (n << 8) + frac_part;
}

static bool
_fspk_probe(void *music, uint16_t length)
{
    spk_header *header = (spk_header *)music;
    return SPK_FORMAT_NOTE3 == header->format;
}

static bool
_fspk_start(void *music, uint16_t length)
{
    spk_header *header = (spk_header *)music;
    _tick_rate = pal_get_ticks(1000 / header->ticks_per_second);
    _sequence = (spk_note3 *)((uint8_t *)music + sizeof(spk_header));
    _ticks = 0;

    char       program = 81; // Lead 1 (square wave)
    midi_event event = {0, MIDI_MSG_PROGRAM, &program, sizeof(program)};
    snd_send(&event);
    return true;
}

static bool
_fspk_step(void)
{
    if (!_sequence)
    {
        errno = EINVAL;
        return false;
    }

    uint32_t now = pal_get_counter();
    if (_next_tick > now)
    {
        return true;
    }

    _next_tick = now + _tick_rate;
    if (0 != _ticks)
    {
        _ticks--;
        return true;
    }

    char msg[] = {_last_note, 64};
    if (SPK_NOTE_DURATION_STOP == _sequence->duration)
    {
        midi_event event = {0, MIDI_MSG_NOTEOFF, msg, sizeof(msg)};
        snd_send(&event);
        _sequence = NULL;
        return true;
    }

    _ticks = _sequence->duration - 1;
    if (0 == _sequence->divisor)
    {
        midi_event event = {0, MIDI_MSG_NOTEOFF, msg, sizeof(msg)};
        snd_send(&event);
    }
    else
    {
        midi_event event = {0, MIDI_MSG_NOTEOFF, msg, sizeof(msg)};
        snd_send(&event);

        // F / d = 440 * 2^((x - 69) / 12)
        // lb(F / (440 * d)) = (x - 69) / 12
        // lb(F / 440) - lb(d) = (x - 69) / 12
        // x = 12 * (lb(F / 440) - lb(d)) + 69
        event.status = MIDI_MSG_NOTEON;
        uint32_t note = NOTE_CONST_U32F8 - 12 * _lb(_sequence->divisor);
        _last_note = ((note >> 8) & 0x7F) + ((note & 0x80) >> 7);
        msg[0] = (char)_last_note;
        snd_send(&event);
    }

    _sequence++;
    return true;
}

snd_format_protocol __snd_fspk = {_fspk_probe, _fspk_start, _fspk_step};
