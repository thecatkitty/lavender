#include <errno.h>

#include <pal.h>
#include <snd.h>

#define INIT_USEC_PER_QUARTER 500000

static iff_context *_iff = 0;
static uint16_t     _division = 960;
static const char  *_track = 0;
static midi_event   _event = {0, NULL, 0};

static uint32_t _kilotick_rate;
static uint32_t _next_event;

static bool
_fmidi_probe(void *music, uint16_t length)
{
    iff_head *head = (iff_head *)music;
    return MIDI_FOURCC_MTHD.dw == head->type.dw;
}

static bool
_fmidi_start(void *music, uint16_t length)
{

    _iff = iff_open(music, length);
    if (NULL == _iff)
    {
        return false;
    }

    midi_mthd mthd;
    iff_chunk mtrk_chunk = {0};

    iff_chunk it = {0};
    while (iff_next_chunk(_iff, &it))
    {
        if (MIDI_FOURCC_MTHD.dw == it.type.dw)
        {
            midi_mthd *data = (midi_mthd *)it.data;

            mthd.format = __builtin_bswap16(data->format);
            mthd.ntrks = __builtin_bswap16(data->ntrks);
            mthd.division = __builtin_bswap16(data->division);
            if (mthd.division & MIDI_MTHD_DIVISION_SMPTE_FLAG)
            {
                // SMPTE timing not supported
                iff_close(_iff);
                errno = EINVAL;
                return false;
            }

            _division = mthd.division;
            _kilotick_rate = pal_get_ticks(INIT_USEC_PER_QUARTER / _division);
        }
        else if (MIDI_FOURCC_MTRK.dw == it.type.dw)
        {
            mtrk_chunk.type = it.type;
            mtrk_chunk.length = it.length;
            mtrk_chunk.data = it.data;
        }

        it.type.dw = 0;
    }

    if (NULL == mtrk_chunk.data)
    {
        // No tracks
        iff_close(_iff);
        errno = EINVAL;
        return false;
    }

    _track = mtrk_chunk.data;
    _next_event = 0;

    return true;
}

static bool
_fmidi_step(void)
{
    if (!_track)
    {
        errno = EINVAL;
        return false;
    }

    uint32_t now = pal_get_counter();
    if (_next_event > now)
    {
        return true;
    }

    if (NULL != _event.msg)
    {
        snd_send(_event.msg, _event.msg_length);
    }

    size_t length = midi_read_event(_track, &_event);
    _track += length;

    while ((0 != length) && (0 == _event.delta) &&
           (_iff->data + _iff->length > _track))
    {
        if ((MIDI_MSG_META == (uint8_t)_event.msg[0]) &&
            (MIDI_META_TEMPO == (uint8_t)_event.msg[1]) &&
            (3 == (uint8_t)_event.msg[2]))
        {
            uint32_t usec_per_quarter = (uint32_t)_event.msg[3] & 0xFF;
            usec_per_quarter <<= 8;
            usec_per_quarter |= (uint32_t)_event.msg[4] & 0xFF;
            usec_per_quarter <<= 8;
            usec_per_quarter |= (uint32_t)_event.msg[5] & 0xFF;
            _kilotick_rate = pal_get_ticks(usec_per_quarter / _division);
        }

        snd_send(_event.msg, _event.msg_length);
        length = midi_read_event(_track, &_event);
        _track += length;
    }

    if (0 == length)
    {
        _track = NULL;
        iff_close(_iff);
        errno = EINVAL;
        return false;
    }

    _next_event = now + _event.delta * _kilotick_rate / 1000;
    return true;
}

snd_format_protocol __snd_fmidi = {_fmidi_probe, _fmidi_start, _fmidi_step};
