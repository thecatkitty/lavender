#ifndef _SND_BUF_H_
#define _SND_BUF_H_

#include <base.h>

typedef struct
{
    size_t   size;
    uint32_t position;
    char    *data;
    int      flags;
} snd_buffer;

#define SNDBUFF_REQPKT (1 << 0)
#define SNDBUFF_ACKPKT (1 << 1)
#define SNDBUFF_NOMORE (1 << 2)

#define SND_PACKET_SIZE(buffer) ((buffer)->size / 2)

extern bool
sndbuf_read(snd_buffer *buffer, size_t size, char *data);

extern int
sndbuf_getc(snd_buffer* buffer);

extern int
sndbuf_peek(snd_buffer* buffer);

extern char *
sndbuf_next_packet(const snd_buffer *buffer);

#endif // _SND_BUF_H_
