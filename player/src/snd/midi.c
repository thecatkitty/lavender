#include <fmt/midi.h>
#include <pal.h>
#include <snd/buf.h>

#define MAX_MSGLEN 1024

#define TRY_GETC(dst, buffer)                                                  \
    {                                                                          \
        int byte = sndbuf_getc(buffer);                                        \
        if (0 > byte)                                                          \
        {                                                                      \
            LOG("cannot get the next byte at %lu!", (long)buffer->position);   \
            return 0;                                                          \
        }                                                                      \
        dst = (char)byte;                                                      \
    }

static char msg_[MAX_MSGLEN];

static void
restore_vlq(char *out, uint32_t value, int size)
{
    int i;

    for (i = size - 1; i >= 0; i--)
    {
        *(uint8_t *)(out + i) = 0x80 | (value & 0x7F);
        value >>= 7;
    }

    *(uint8_t *)(out + size - 1) &= 0x7F;
}

size_t
midi_read_event(snd_buffer *buffer, midi_event *event)
{
    int    status;
    size_t size = 0;

    int delta_len = midi_read_vlq(buffer, &event->delta);
    if (0 > delta_len)
    {
        return 0;
    }

    size += delta_len;

    status = sndbuf_peek(buffer);
    if (0x80 <= status)
    {
        status = sndbuf_getc(buffer);
        if (0 > status)
        {
            return 0;
        }

        event->status = (uint8_t)status;
        size++;
    }
    else
    {
        status = event->status;
    }

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
        event->msg_length = 0;
        break;

    case MIDI_MSG_PROGRAM:
    case MIDI_MSG_CHANPRESS:
    case MIDI_MSG_ENDEX:
    case MIDI_MSG_SONG: {
        event->msg_length = 1;
        TRY_GETC(msg_[0], buffer);
        event->msg = msg_;
        break;
    }

    case MIDI_MSG_NOTEOFF:
    case MIDI_MSG_NOTEON:
    case MIDI_MSG_POLYPRESS:
    case MIDI_MSG_CONTROL:
    case MIDI_MSG_PITCHWHEEL:
    case MIDI_MSG_POSITION:
        event->msg_length = 2;
        TRY_GETC(msg_[0], buffer);
        TRY_GETC(msg_[1], buffer);
        event->msg = msg_;
        break;

    case MIDI_MSG_SYSEX: {
        int i = 0;

        while (MIDI_MSG_ENDEX != sndbuf_peek(buffer))
        {
            if (MAX_MSGLEN == event->msg_length)
            {
                LOG("SysEx message is too long!");
                return 0;
            }

            TRY_GETC(msg_[i], buffer);
            event->msg_length++;
            i++;
        }

        event->msg = msg_;
        break;
    }

    case MIDI_MSG_META: {
        uint32_t meta_vlq;
        int      meta_vlq_len;

        TRY_GETC(msg_[0], buffer);
        size++;

        meta_vlq_len = midi_read_vlq(buffer, &meta_vlq);
        if ((0 == meta_vlq_len) || ((MAX_MSGLEN - 1 - meta_vlq_len) < meta_vlq))
        {
            LOG("MetaEvent message is too long (%lu)!", (long)meta_vlq);
            return 0;
        }

        event->msg_length = 1 + meta_vlq_len + meta_vlq;

        restore_vlq(msg_ + 1, meta_vlq, meta_vlq_len);
        if (!sndbuf_read(buffer, meta_vlq, msg_ + 1 + meta_vlq_len))
        {
            LOG("cannot load the MetaEvent message!");
            return 0;
        }

        event->msg = msg_;
        break;
    }

    default:
        return 0;
    }

    return size + event->msg_length;
}

size_t
midi_read_vlq(snd_buffer *buffer, uint32_t *vlq)
{
    uint32_t out = 0;

    int byte, count = 0;
    while (5 > count)
    {
        byte = sndbuf_getc(buffer);
        if (0 > byte)
        {
            return 0;
        }

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
