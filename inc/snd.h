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
extern bool
snd_play(const char *name);

// Handle sound related services
extern void
snd_handle(void);

// Send MIDI message
extern void
snd_send(midi_event *event);

#endif // _SND_H_
