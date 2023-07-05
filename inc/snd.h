#ifndef _SND_H_
#define _SND_H_

#include <base.h>
#include <fmt/midi.h>

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
