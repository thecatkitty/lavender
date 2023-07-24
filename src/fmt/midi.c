#include <fmt/midi.h>
#include <pal.h>
#include <snd.h>

#define USEC_PER_QUARTER  500000
#define TICKS_PER_QUARTER 960
#define USEC_PER_TICK     (USEC_PER_QUARTER / TICKS_PER_QUARTER)

size_t
midi_read_event(const char *buffer, midi_event *event)
{
    const char *ptr = buffer;

    int delta_len = midi_read_vlq(ptr, &event->delta);
    if (0 > delta_len)
    {
        return 0;
    }

    ptr += delta_len;
    event->msg = ptr;

    uint8_t status = *ptr;
    ptr++;

    if (MIDI_MSG_SYSEX > status)
    {
        status &= 0xF0;
    }

    switch (status)
    {
    case MIDI_MSG_TUNE:
    case MIDI_MSG_CLOCK:
    case MIDI_MSG_START:
    case MIDI_MSG_CONTINUE:
    case MIDI_MSG_STOP:
    case MIDI_MSG_SENSING:
        break;

    case MIDI_MSG_PROGRAM:
    case MIDI_MSG_CHANPRESS:
    case MIDI_MSG_ENDEX:
    case MIDI_MSG_SONG:
        ptr += 1;
        break;

    case MIDI_MSG_NOTEOFF:
    case MIDI_MSG_NOTEON:
    case MIDI_MSG_POLYPRESS:
    case MIDI_MSG_CONTROL:
    case MIDI_MSG_PITCHWHEEL:
    case MIDI_MSG_POSITION:
        ptr += 2;
        break;

    case MIDI_MSG_SYSEX: {
        int mfg = *ptr;
        ptr++;

        if (0 == mfg)
        {
            ptr += 2;
        }

        while (MIDI_MSG_ENDEX != (uint8_t)*ptr)
        {
            ptr++;
        }
        break;
    }

    case MIDI_MSG_META: {
        ptr++;

        uint32_t meta_vlq;
        int      meta_vlq_len = midi_read_vlq(ptr, &meta_vlq);
        if (0 > meta_vlq_len)
        {
            return 0;
        }

        ptr += meta_vlq_len + meta_vlq;
        break;
    }

    default:
        return 0;
    }

    event->msg_length = ptr - buffer - delta_len;
    return ptr - buffer;
}

size_t
midi_read_vlq(const char *buffer, uint32_t *vlq)
{
    uint32_t out = 0;

    int byte, count = 0;
    while (5 > count)
    {
        byte = buffer[count];
        count++;

        out <<= 7;
        out |= byte & 0x7F;
        if (0 == (byte & 0x80))
        {
            break;
        }
    }

    if (4 < count)
    {
        return 0;
    }

    *vlq = out;
    return count;
}
