#ifndef _SND_H_
#define _SND_H_

#include <dev.h>
#include <fmt/midi.h>

typedef struct
{
    bool ddcall (*open)(device *dev);
    void ddcall (*close)(device *dev);
    bool ddcall (*write)(device *dev, const midi_event *);
    bool ddcall (*tick)(device *dev, uint32_t ts);
} snd_device_ops;

#define snd_device_open(dev) (((far snd_device_ops *)((dev)->ops))->open((dev)))
#define snd_device_close(dev)                                                  \
    (((far snd_device_ops *)((dev)->ops))->close((dev)))
#define snd_device_write(dev, event)                                           \
    (((far snd_device_ops *)((dev)->ops))->write((dev), (event)))
#define snd_device_tick(dev, ts)                                               \
    (((far snd_device_ops *)((dev)->ops))->tick((dev), (ts)))

typedef bool (*snd_enum_devices_callback)(device *device, void *data);

// Load inbox drivers
extern void
snd_load_inbox_drivers(void);

// Enumerate playback devices
extern void
snd_enum_devices(snd_enum_devices_callback callback, void *data);

// Register a sound device
extern int ddcall
snd_register_device(far device *dev);

// Unregister all sound devices sharing one protocol
extern int ddcall
snd_unregister_devices(far snd_device_ops *ops);

// Initialize sound system
extern bool
snd_initialize(const char *arg);

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

// Start MIDI sequence
extern bool
sndseq_start(void *music, uint16_t length);

// Step MIDI sequence forward
extern bool
sndseq_step(void);

#endif // _SND_H_
