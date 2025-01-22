#include <string.h>

#ifdef CONFIG_ANDREA
#include <andrea.h>
#include <arch/dos.h>
#endif

#include <generated/config.h>

#include <pal.h>
#include <snd.h>

#define MAX_DEVICES 4

static device   _devices[MAX_DEVICES] = {{{0}}};
static device  *_device = NULL;
static hasset   _music = NULL;
static uint32_t _ts = 0;

#if defined(CONFIG_ANDREA)
static uint16_t _driver = 0;
#else
extern int ddcall
__mpu401_init(void);

extern int ddcall
__opl2_init(void);
#endif

extern int ddcall
__beep_init(void);

void
snd_load_inbox_drivers(void)
{
#if defined(__ia16__)
#if !defined(CONFIG_ANDREA)
    __mpu401_init();
    __opl2_init();
#endif // CONFIG_ANDREA
    __beep_init();
#endif // __ia16__
}

void
snd_enum_devices(snd_enum_devices_callback callback, void *data)
{
    int i;
    for (i = 0; i < MAX_DEVICES; i++)
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
}

int ddcall
snd_register_device(far device *dev)
{
    int i;
    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (NULL == _devices[i].ops)
        {
            _fmemcpy(_devices + i, dev, sizeof(*dev));
            return 0;
        }
    }

    errno = ENOMEM;
    return -errno;
}

int ddcall
snd_unregister_devices(far snd_device_ops *ops)
{
    int count = 0, i;

    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (ops == _devices[i].ops)
        {
            _devices[i].ops = NULL;
            count++;
        }
    }

    return count;
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(snd_register_device);
ANDREA_EXPORT(snd_unregister_devices);
#endif

static bool
_try_open(device *dev, void *data)
{
    const char *arg = (const char *)data;

    if (arg && (0 != strcasecmp(arg, dev->name)))
    {
        return true;
    }

    if (snd_device_open(dev))
    {
        _device = dev;
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
    uint16_t driver = dos_load_driver(name);
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

    dos_unload_driver(driver);
    return true;
}
#endif // CONFIG_ANDREA

bool
snd_initialize(const char *arg)
{
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
}

void
snd_cleanup(void)
{
    if (_device)
    {
        snd_device_close(_device);
    }

#if defined(CONFIG_ANDREA)
    if (0 != _driver)
    {
        dos_unload_driver(_driver);
    }
#endif // CONFIG_ANDREA
}

bool
snd_send(const midi_event *event)
{
    if (NULL == _device)
    {
        return false;
    }

    return snd_device_write(_device, event);
}

void
snd_handle(void)
{
    if ((NULL == _device) || (NULL == _music))
    {
        return;
    }

    if (!sndseq_step())
    {
        pal_close_asset(_music);
        _music = NULL;
    }

    if (NULL != ((far snd_device_ops *)_device->ops)->tick)
    {
        uint32_t ts = pal_get_counter();
        if (_ts != ts)
        {
            _ts = ts;
            snd_device_tick(_device, ts);
        }
    }
}

bool
snd_play(const char *name)
{
    char *data;
    long  length;

    if (_music)
    {
        pal_close_asset(_music);
    }

    _music = pal_open_asset(name, O_RDONLY);
    if (NULL == _music)
    {
        return false;
    }

    data = pal_get_asset_data(_music);
    if (NULL == data)
    {
        return false;
    }

    length = pal_get_asset_size(_music);
    if (sndseq_start(data, length))
    {
        return true;
    }

    pal_close_asset(_music);
    _music = NULL;
    return false;
}
