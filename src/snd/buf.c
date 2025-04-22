#include <pal.h>
#include <snd/buf.h>

static bool
request_packet(snd_buffer *buffer)
{
    LOG("entry at %lu", (unsigned long)buffer->position);

    if (buffer->flags & SNDBUFF_NOMORE)
    {
        LOG("no more packets after %lu!", (unsigned long)buffer->position);
        return true;
    }

    if ((buffer->flags & SNDBUFF_REQPKT) && !(buffer->flags & SNDBUFF_ACKPKT))
    {
        LOG("underrun at %lu!", (unsigned long)buffer->position);
        return false;
    }

    buffer->flags &= ~SNDBUFF_ACKPKT;
    buffer->flags |= SNDBUFF_REQPKT;
    return true;
}

bool
sndbuf_read(snd_buffer *buffer, size_t size, char *data)
{
    int    i;
    size_t packet = SND_PACKET_SIZE(buffer);
    size_t position = (size_t)(buffer->position % (uint32_t)buffer->size);

    for (i = 0; i < size; i++)
    {
        data[i] = buffer->data[position];

        position++;
        buffer->position++;

        if (packet == position)
        {
            if (!request_packet(buffer))
            {
                break;
            }
        }

        if (buffer->size == position)
        {
            position = 0;
            if (!request_packet(buffer))
            {
                break;
            }
        }
    }

    return size == i;
}

int
sndbuf_getc(snd_buffer* buffer)
{
    uint8_t byte = 0;

    if (!sndbuf_read(buffer, 1, (char*)&byte))
    {
        return -1;
    }

    return byte;
}

int
sndbuf_peek(snd_buffer* buffer)
{
    size_t position = (size_t)(buffer->position % (uint32_t)buffer->size);
    return (uint8_t)buffer->data[position];
}

extern char *
sndbuf_next_packet(const snd_buffer *buffer)
{
    size_t packet = SND_PACKET_SIZE(buffer);
    size_t position = (size_t)(buffer->position % (uint32_t)buffer->size);
    return buffer->data + packet * (1 - position / packet);
}
