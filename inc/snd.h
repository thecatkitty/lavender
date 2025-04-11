#ifndef _SND_H_
#define _SND_H_

#include <dev.h>

typedef bool (*snd_enum_devices_callback)(device *device, void *data);

// Load inbox drivers
extern void
snd_load_inbox_drivers(void);

// Enumerate playback devices
extern void
snd_enum_devices(snd_enum_devices_callback callback, void *data);

// Initialize sound system
extern bool
snd_initialize(const char *arg);

// Clean up sound system
extern void
snd_cleanup(void);

// Get currently active device pointer
extern device *
snd_get_device(void);

// Start playing music
extern bool
snd_play(const char *name);

// Handle sound related services
extern void
snd_handle(void);

#endif // _SND_H_
