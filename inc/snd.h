#ifndef _SND_H_
#define _SND_H_

#include <fmt/midi.h>

typedef struct
{
    bool (*probe)(void *, uint16_t);
    bool (*start)(void *, uint16_t);
    bool (*step)(void);
} snd_format_protocol;

// Start playing music
extern void
snd_play(void *music, uint16_t length);

// Handle sound related services
extern void
snd_handle(void);

// Send MIDI message
extern void
snd_send(const char *msg, size_t length);

#endif // _SND_H_
