#include <string.h>

#include <pal.h>
#include <snd.h>

extern snd_device_protocol __snd_dfluid;
extern snd_device_protocol __snd_dmme;
extern snd_device_protocol __snd_dmpu401;
extern snd_device_protocol __snd_dpcspk;

extern snd_format_protocol __snd_fmidi;
extern snd_format_protocol __snd_fspk;

static snd_device_protocol *_devices[] = {
#ifdef __MINGW32__
    &__snd_dmme,
#endif
#ifdef __linux__
    &__snd_dfluid,
#endif
#if !defined(__MINGW32__) && !defined(__linux__)
    &__snd_dmpu401,
    &__snd_dpcspk,
#endif
};

static snd_format_protocol *_formats[] = {
    &__snd_fmidi, // Standard MIDI File, type 0
    &__snd_fspk   // length-divisor pairs for PC Speaker
};

static snd_device_protocol _device = {NULL, NULL, NULL};
static snd_format_protocol _format;
static hasset              _music = NULL;

void
snd_enum_devices(snd_enum_devices_callback callback, void *data)
{
    for (snd_device_protocol **device = _devices;
         device < _devices + lengthof(_devices); device++)
    {
        if (!callback(*device, data))
        {
            return;
        }
    }
}

static bool
_try_open(snd_device_protocol *device, void *data)
{
    const char *arg = (const char *)data;

    if (arg && (0 != strcasecmp(arg, device->name)))
    {
        return true;
    }

    if (device->open())
    {
        _device = *device;
        return false;
    }

    if (arg)
    {
        return false;
    }

    return true;
}

bool
snd_initialize(const char *arg)
{
    snd_enum_devices(_try_open, (void *)arg);
    return NULL != _device.open;
}

void
snd_cleanup(void)
{
    if (_device.close)
    {
        _device.close();
    }
}

bool
snd_send(const midi_event *event)
{
    if (NULL == _device.write)
    {
        return false;
    }

    return _device.write(event);
}

void
snd_handle(void)
{
    if (_format.step && _music)
    {
        if (!_format.step())
        {
            pal_close_asset(_music);
            _music = NULL;
        }
    }
}

bool
snd_play(const char *name)
{
    _format.probe = 0;
    _format.start = 0;
    _format.step = 0;

    if (_music)
    {
        pal_close_asset(_music);
    }

    _music = pal_open_asset(name, O_RDONLY);
    if (NULL == _music)
    {
        return false;
    }

    char *data = pal_get_asset_data(_music);
    if (NULL == data)
    {
        return false;
    }

    int length = pal_get_asset_size(_music);
    for (snd_format_protocol **format = _formats;
         format < _formats + lengthof(_formats); format++)
    {
        if ((*format)->probe(data, length))
        {
            _format = **format;
            break;
        }
    }

    if (_format.start)
    {
        _format.start(data, length);
        return true;
    }

    pal_close_asset(_music);
    _music = NULL;
    return false;
}
