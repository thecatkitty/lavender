#ifndef _SND_DEV_H_
#define _SND_DEV_H_

#include <dev.h>
#include <fmt/midi.h>
#include <snd.h>

typedef struct
{
    bool ddcall (*open)(device *dev);
    void ddcall (*close)(device *dev);
    bool ddcall (*write)(device *dev, const midi_event *);
    bool ddcall (*tick)(device *dev, uint32_t ts);
} snd_device_ops;

#define SND_DEVICE_OPS(dev)          ((far snd_device_ops *)((dev)->ops))
#define snd_device_open(dev)         SND_DEVICE_OPS(dev)->open((dev))
#define snd_device_close(dev)        SND_DEVICE_OPS(dev)->close((dev))
#define snd_device_write(dev, event) SND_DEVICE_OPS(dev)->write((dev), (event))
#define snd_device_tick(dev, ts)     SND_DEVICE_OPS(dev)->tick((dev), (ts))

// Register a sound device
extern int ddcall
snd_register_device(far device *dev);

// Unregister all sound devices sharing one protocol
extern int ddcall
snd_unregister_devices(far snd_device_ops *ops);

#endif // _SND_DEV_H_
