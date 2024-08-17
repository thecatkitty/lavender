#ifndef _SND_H_
#define _SND_H_

#include <fmt/midi.h>

#define SND_MAX_NAME        9
#define SND_MAX_DESCRIPTION 32

#ifdef LOADABLE
#include <andrea.h>

#define DRV_DATA ANDREA_MODDATA
#define DRV_RDAT __attribute__((section(".text.rdat"))) far
#define DRV_INIT(name)                                                         \
    extern int ddcall drv_init(void);                                          \
    ANDREA_EXPORT(drv_init);                                                   \
    int ddcall drv_init
#else
#define DRV_DATA
#define DRV_RDAT
#define DRV_INIT(name) int ddcall __##name##_init
#endif

typedef struct _snd_device snd_device;

typedef struct
{
    bool ddcall (*open)(snd_device *dev);
    void ddcall (*close)(snd_device *dev);
    bool ddcall (*write)(snd_device *dev, const midi_event *);
} snd_device_ops;

typedef struct _snd_device
{
    char name[SND_MAX_NAME];
    char description[SND_MAX_DESCRIPTION];

    far snd_device_ops *ops;
    far void           *data;
} snd_device;

#define snd_device_open(dev)         ((dev)->ops->open((dev)))
#define snd_device_close(dev)        ((dev)->ops->close((dev)))
#define snd_device_write(dev, event) ((dev)->ops->write((dev), (event)))

typedef struct
{
    bool (*probe)(void *, uint16_t);
    bool (*start)(void *, uint16_t);
    bool (*step)(void);
} snd_format_protocol;

typedef bool (*snd_enum_devices_callback)(snd_device *device, void *data);

// Enumerate playback devices
extern void
snd_enum_devices(snd_enum_devices_callback callback, void *data);

// Register a sound device
extern int ddcall
snd_register_device(far snd_device *dev);

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

#endif // _SND_H_
