#ifndef _SND_H_
#define _SND_H_

#include <fmt/midi.h>

typedef struct
{
    bool (*open)(void);
    void (*close)(void);
    bool (*write)(const midi_event *);
} snd_device_protocol;

typedef struct
{
    bool (*probe)(void *, uint16_t);
    bool (*start)(void *, uint16_t);
    bool (*step)(void);
} snd_format_protocol;

// Initialize sound system
extern bool
snd_initialize(void);

// Clean up sound system
extern void
snd_cleanup(void);

// Start playing music
extern bool
snd_play(const char *name);

// Handle sound related services
extern void
snd_handle(void);

// Send MIDI message
extern bool
snd_send(const midi_event *event);

#endif // _SND_H_
