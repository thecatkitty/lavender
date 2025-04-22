#include <pal.h>
#include <snd.h>
#include <snd/buf.h>
#include <snd/dev.h>
#include <snd/seq.h>

#define BUFFER_SIZE           4096
#define INIT_USEC_PER_QUARTER 500000

static device      *_dev = NULL;
static hasset       _asset = NULL;
static iff_context *_iff = 0;
static uint16_t     _division = 960;
static snd_buffer   _buffer = {0, 0, NULL, 0};
static uint32_t     _track_start = 0, _track_length = 0;
static midi_event   _event = {0, 0, NULL, 0};

static uint32_t _kilotick_rate;
static uint32_t _next_event;

bool
sndseq_open(const char *name)
{
    hasset asset;

    if (_asset)
    {
        asset = _asset;
        _asset = NULL;
        pal_close_asset(asset);
    }

    asset = pal_open_asset(name, O_RDONLY);
    if (NULL == asset)
    {
        return false;
    }

    _asset = asset;
    return true;
}

bool
sndseq_close(void)
{
    char *buffer_data = _buffer.data;
    _buffer.data = NULL;
    free(buffer_data);

    if (NULL == _asset)
    {
        return false;
    }

    pal_close_asset(_asset);
    _asset = NULL;
    return true;
}

bool
sndseq_start(void)
{
    iff_chunk mtrk_chunk = {0};
    iff_chunk it = {0};

    _iff = iff_open(_asset);
    if (NULL == _iff)
    {
        return false;
    }

    if (MIDI_FOURCC_MTHD.dw != _iff->type.dw)
    {
        iff_close(_iff);
        errno = EFTYPE;
        return false;
    }

    while (iff_next_chunk(_iff, &it))
    {
        if (MIDI_FOURCC_MTHD.dw == it.type.dw)
        {
            midi_mthd mthd;
            pal_read_asset(_iff->asset, (char *)&mthd, it.position,
                           sizeof(mthd));
            mthd.format = BSWAP16(mthd.format);
            mthd.ntrks = BSWAP16(mthd.ntrks);
            mthd.division = BSWAP16(mthd.division);
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
            mtrk_chunk.position = it.position;
        }

        it.type.dw = 0;
    }

    if (0 == mtrk_chunk.position)
    {
        // No tracks
        iff_close(_iff);
        errno = EINVAL;
        return false;
    }

    _buffer.size = BUFFER_SIZE;
    _buffer.position = 0;
    _buffer.flags = 0;
    _buffer.data = (char *)malloc(_buffer.size);
    if (NULL == _buffer.data)
    {
        return false;
    }

    _track_start = mtrk_chunk.position;
    _track_length = mtrk_chunk.length;
    pal_read_asset(_iff->asset, _buffer.data, _track_start, _buffer.size);

    _dev = snd_get_device();

    return true;
}

bool
sndseq_step(void)
{
    uint32_t now;
    size_t   length;

    if (NULL == _buffer.data)
    {
        LOG("no allocated buffer!");
        errno = EINVAL;
        return false;
    }

    if (_track_length <= _buffer.position)
    {
        char *data = _buffer.data;
        _buffer.data = NULL;
        free(data);
        iff_close(_iff);

        LOG("finished at %lu", _buffer.position);
        return true;
    }

    now = pal_get_counter();
    if (_next_event > now)
    {
        return true;
    }

    if (NULL != _event.msg)
    {
        snd_device_write(_dev, &_event);
    }

    length = midi_read_event(&_buffer, &_event);

    while ((0 != length) && (0 == _event.delta) &&
           (_track_length > _buffer.position))
    {
        if ((MIDI_MSG_META == (uint8_t)_event.status) && (NULL != _event.msg) &&
            (MIDI_META_TEMPO == (uint8_t)_event.msg[0]) &&
            (3 == (uint8_t)_event.msg[1]))
        {
            uint32_t usec_per_quarter = (uint32_t)_event.msg[2] & 0xFF;
            usec_per_quarter <<= 8;
            usec_per_quarter |= (uint32_t)_event.msg[3] & 0xFF;
            usec_per_quarter <<= 8;
            usec_per_quarter |= (uint32_t)_event.msg[4] & 0xFF;
            _kilotick_rate = pal_get_ticks(usec_per_quarter / _division);
        }

        snd_device_write(_dev, &_event);
        length = midi_read_event(&_buffer, &_event);
    }

    if (0 == length)
    {
        char *data = _buffer.data;
        _buffer.data = NULL;
        free(data);
        iff_close(_iff);
        errno = EINVAL;

        LOG("cannot read event at %lu!", _buffer.position);
        return false;
    }

    _next_event = now + _event.delta * _kilotick_rate / 1000;
    return true;
}

static uint32_t
get_next_packet_start(const snd_buffer *buf)
{
    size_t packet = SND_PACKET_SIZE(&_buffer);
    return packet * (_buffer.position / packet + 1);
}

bool
sndseq_feed(void)
{
    uint32_t packet_start;
    size_t   packet_size;
    if (SNDBUFF_REQPKT !=
        (_buffer.flags & (SNDBUFF_REQPKT | SNDBUFF_ACKPKT | SNDBUFF_NOMORE)))
    {
        return true;
    }

    packet_start = get_next_packet_start(&_buffer);
    packet_size = SND_PACKET_SIZE(&_buffer);
    if (_track_length < (packet_start + packet_size))
    {
        if (_track_length < packet_start)
        {
            LOG("end of track!");
            _buffer.flags |= SNDBUFF_NOMORE;
            return true;
        }

        packet_size = _track_length - packet_start;
    }

    LOG("packet requested, %lu @ %lu", (long)packet_size, (long)packet_start);

    if (!pal_read_asset(_iff->asset, sndbuf_next_packet(&_buffer),
                        _track_start + packet_start, packet_size))
    {
        LOG("read error at %lu!", _buffer.position);
        return false;
    }

    _buffer.flags |= SNDBUFF_ACKPKT;
    return true;
}
