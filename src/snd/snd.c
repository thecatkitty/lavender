#include <string.h>

#ifdef CONFIG_ANDREA
#include <andrea.h>
#include <platform/dospc.h>
#endif

#include <generated/config.h>

#include <pal.h>
#include <snd.h>

#ifdef __ia16__
#include <libi86/string.h>
#define fmemcpy _fmemcpy
#else
#define fmemcpy memcpy
#endif

#define MAX_DEVICES 4

#if defined(CONFIG_SOUND)
extern snd_format_protocol __snd_fmidi;
extern snd_format_protocol __snd_fspk;

static snd_device           _devices[MAX_DEVICES] = {{{0}}};
static snd_format_protocol *_formats[] = {
    &__snd_fmidi, // Standard MIDI File, type 0
    &__snd_fspk   // length-divisor pairs for PC Speaker
};

static snd_device         *_device = NULL;
static snd_format_protocol _format;
static hasset              _music = NULL;

#if defined(CONFIG_ANDREA)
static uint16_t _driver = 0;
#else
extern int ddcall
__mpu401_init(void);

extern int ddcall
__opl2_init(void);
#endif

extern int ddcall
__pcspk_init(void);
#endif // CONFIG_SOUND

void
snd_load_inbox_drivers(void)
{
#if defined(__ia16__)
#if !defined(CONFIG_ANDREA)
    __mpu401_init();
    __opl2_init();
#endif // CONFIG_ANDREA
    __pcspk_init();
#endif // __ia16__
}

void
snd_enum_devices(snd_enum_devices_callback callback, void *data)
{
#if defined(CONFIG_SOUND)
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (NULL == _devices[i].ops)
        {
            continue;
        }

        if (!callback(_devices + i, data))
        {
            return;
        }
    }
#endif // CONFIG_SOUND
}

int ddcall
snd_register_device(far snd_device *dev)
{
#if defined(CONFIG_SOUND)
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (NULL == _devices[i].ops)
        {
            fmemcpy(_devices + i, dev, sizeof(*dev));
            return 0;
        }
    }

    errno = ENOMEM;
#else  // CONFIG_SOUND
    errno = ENOSYS;
#endif // CONFIG_SOUND
    return -errno;
}

int ddcall
snd_unregister_devices(far snd_device_ops *ops)
{
#if defined(CONFIG_SOUND)
    int count = 0;

    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (ops == _devices[i].ops)
        {
            _devices[i].ops = NULL;
            count++;
        }
    }

    return count;
#else  // CONFIG_SOUND
    errno = ENOSYS;
    return -errno;
#endif // CONFIG_SOUND
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(snd_register_device);
ANDREA_EXPORT(snd_unregister_devices);
#endif

#if defined(CONFIG_SOUND)
static bool
_try_open(snd_device *device, void *data)
{
    const char *arg = (const char *)data;

    if (arg && (0 != strcasecmp(arg, device->name)))
    {
        return true;
    }

    if (snd_device_open(device))
    {
        _device = device;
        return false;
    }

    if (arg)
    {
        return false;
    }

    return true;
}

#if defined(CONFIG_ANDREA)
static bool
_try_driver(const char *name, void *data)
{
    uint16_t driver = dospc_load_driver(name);
    if (0 == driver)
    {
        return true;
    }

    snd_enum_devices(_try_open, data);
    if (NULL != _device)
    {
        _driver = driver;
        return false;
    }

    dospc_unload_driver(driver);
    return true;
}
#endif // CONFIG_ANDREA
#endif // CONFIG_SOUND

bool
snd_initialize(const char *arg)
{
#if defined(CONFIG_SOUND)
#if defined(CONFIG_ANDREA)
    if (NULL == _device)
    {
        pal_enum_assets(_try_driver, "snd*.sys", (void *)arg);
    }
#endif // CONFIG_ANDREA

    if (NULL == _device)
    {
        snd_load_inbox_drivers();
        snd_enum_devices(_try_open, (void *)arg);
    }

    return NULL != _device;
#else
    return false;
#endif // CONFIG_SOUND
}

void
snd_cleanup(void)
{
#if defined(CONFIG_SOUND)
    if (_device)
    {
        snd_device_close(_device);
    }

#if defined(CONFIG_ANDREA)
    if (0 != _driver)
    {
        dospc_unload_driver(_driver);
    }
#endif // CONFIG_ANDREA
#endif // CONFIG_SOUND
}

bool
snd_send(const midi_event *event)
{
#if defined(CONFIG_SOUND)
    if (NULL == _device)
    {
        return false;
    }

    return snd_device_write(_device, event);
#else
    return false;
#endif // CONFIG_SOUND
}

void
snd_handle(void)
{
#if defined(CONFIG_SOUND)
    if (_format.step && _music)
    {
        if (!_format.step())
        {
            pal_close_asset(_music);
            _music = NULL;
        }
    }
#endif // CONFIG_SOUND
}

bool
snd_play(const char *name)
{
#if defined(CONFIG_SOUND)
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
#else
    errno = ENOSYS;
#endif // CONFIG_SOUND
    return false;
}
