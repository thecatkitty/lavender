#include <drv.h>
#include <pal.h>
#include <platform/dospc.h>
#include <snd.h>

#define KEY_NONE         255
#define KEY_MIN          14 // PIT incapable of frequencies lower than D0
#define KEY_A4           69
#define FREQ_A4          440
#define INPUT_FREQ_U32F8 305454507UL

#define VOICES   3
#define SWEEP_MS 16

// As 8-bit fractional part; integer part always 1
static const uint8_t DRV_RDAT KEY_MULTIPLIERS[] = {
    0,   // A   .000
    15,  // A#  .059
    31,  // B   .122
    48,  // C   .189
    67,  // C#  .260
    86,  // D   .335
    106, // D#  .414
    128, // E   .498
    150, // F   .587
    175, // F#  .682
    200, // G   .782
    227  // G#  .888
};

typedef struct
{
    uint8_t  key[VOICES];
    uint16_t div[VOICES];
    unsigned voice;
    uint32_t last_ts;
    uint32_t sweep;
} beep_data;

#define get_data(dev) ((far beep_data *)((dev)->data))

static unsigned
_find_slot(far beep_data *data, uint8_t key)
{
    unsigned i;

    for (i = 0; i < VOICES; i++)
    {
        if (key == data->key[i])
        {
            break;
        }
    }

    return i;
}

static bool ddcall
beep_open(device *dev)
{
    if (NULL == (dev->data = _fmalloc(sizeof(beep_data))))
    {
        return false;
    }

    _fmemset(dev->data, 0, sizeof(beep_data));

    far beep_data *data = get_data(dev);
    for (unsigned i = 0; i < VOICES; i++)
    {
        data->key[i] = KEY_NONE;
    }

    data->sweep = pal_get_ticks(SWEEP_MS);
    return true;
}

static void ddcall
beep_close(device *dev)
{
    if (NULL != dev->data)
    {
        _ffree(dev->data);
        dev->data = NULL;
    }
}

static bool ddcall
beep_write(device *dev, const midi_event *event)
{
    far beep_data *data = get_data(dev);

    uint8_t     status = event->status;
    const char *msg = event->msg;

    uint8_t channel = status & 0xF;
    if (0xF0 != (status & 0xF0))
    {
        status &= 0xF0;
    }
    else
    {
        channel = 0;
    }

    if ((VOICES <= channel) || (MIDI_DRUMS_CHANNEL == channel))
    {
        return true;
    }

    switch (status)
    {
    case MIDI_MSG_NOTEOFF:
    case MIDI_MSG_NOTEON: {
        uint8_t key = msg[0];
        uint8_t velocity = msg[1];

        if ((MIDI_MAX_KEY < key) || (MIDI_MAX_VELOCITY < velocity))
        {
            return false;
        }

        unsigned slot = VOICES;
        if ((MIDI_MSG_NOTEOFF == status) || (0 == velocity))
        {
            if (VOICES != (slot = _find_slot(data, key)))
            {
                data->key[slot] = KEY_NONE;
            }
            return true;
        }

        if (KEY_MIN > key)
        {
            return false;
        }

        if (VOICES == (slot = _find_slot(data, KEY_NONE)))
        {
            return true;
        }

        uint32_t freq = FREQ_A4;
        if (KEY_A4 < key)
        {
            uint8_t delta = key - KEY_A4;
            freq <<= delta / 12;
            freq *= 256 + KEY_MULTIPLIERS[delta % 12];
        }
        else
        {
            uint8_t delta = KEY_A4 - key;
            freq <<= 16;
            freq >>= delta / 12;
            freq /= 256 + KEY_MULTIPLIERS[delta % 12];
        }

        // freq is u32f8
        data->key[slot] = key;
        data->div[slot] = INPUT_FREQ_U32F8 / freq;
        break;
    }
    }

    return true;
}

static bool ddcall
beep_tick(device *dev, uint32_t ts)
{
    far beep_data *data = get_data(dev);

    bool is_silent = true;
    for (unsigned i = 0; i < VOICES; i++)
    {
        if (KEY_NONE != data->key[i])
        {
            is_silent = false;
            break;
        }
    }

    if (is_silent)
    {
        dospc_silence();
        data->voice = 0;
        return true;
    }

    if (data->sweep > (ts - data->last_ts))
    {
        return true;
    }

    data->last_ts = ts;
    dospc_beep(data->div[data->voice]);
    do
    {
        data->voice = (data->voice + 1) % VOICES;
    } while (KEY_NONE == data->key[data->voice]);
    return true;
}

static device DRV_DATA         _dev = {"beep", "PC Speaker"};
static snd_device_ops DRV_DATA _ops = {beep_open, beep_close, beep_write,
                                       beep_tick};

DRV_INIT(beep)(void)
{
    _dev.ops = &_ops;
    return snd_register_device(&_dev);
}

#ifdef LOADABLE
int ddcall
drv_deinit(void)
{
    return snd_unregister_devices(&_ops);
}

ANDREA_EXPORT(drv_deinit);
#endif

#if defined(__linux__)
snd_device_ops *__beep_ops = &_ops;
#endif
