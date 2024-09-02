#include <SDL2/SDL.h>
#include <fluidsynth.h>

#include <pal.h>
#include <snd.h>

static const char *SOUNDFONTS[] = {"/usr/share/sounds/sf2/default-GM.sf2",
                                   "/usr/share/soundfonts/default-GM.sf2",
                                   "/usr/share/sounds/sf2/default.sf2",
                                   "/usr/share/soundfonts/default.sf2", NULL};

static fluid_settings_t     *_settings;
static fluid_synth_t        *_synth;
static fluid_audio_driver_t *_audio;

static snd_device      _beep;
extern snd_device_ops *__beep_ops;

extern void
beepemu_stop(void);

static bool ddcall
fluid_open(snd_device *dev)
{
    LOG("entry");

    if (0 > SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        LOG("cannot initialize audio subsystem. %s", SDL_GetError());
        return false;
    }

    _settings = new_fluid_settings();
    if (NULL == _settings)
    {
        LOG("cannot create FluidSynth settings");
        return false;
    }

    fluid_settings_setstr(_settings, "audio.driver", "sdl2");

    _synth = new_fluid_synth(_settings);
    if (NULL == _synth)
    {
        LOG("cannot create the synthesizer");
        return false;
    }

    int sfont = FLUID_FAILED;
    for (const char **path = SOUNDFONTS; *path; path++)
    {
        LOG("trying SoundFont '%s'", *path);
        if (FLUID_FAILED != (sfont = fluid_synth_sfload(_synth, *path, 1)))
        {
            break;
        }
    }

    if (FLUID_FAILED == sfont)
    {
        LOG("cannot load any SoundFont");
        delete_fluid_synth(_synth);
        _synth = NULL;
        return false;
    }

    _audio = new_fluid_audio_driver(_settings, _synth);
    if (NULL == _audio)
    {
        LOG("cannot create the audio driver");
        delete_fluid_synth(_synth);
        _synth = NULL;
        return false;
    }

    _beep.ops = __beep_ops;
    if (!snd_device_open(&_beep))
    {
        _beep.ops = NULL;
    }

    return true;
}

static void ddcall
fluid_close(snd_device *dev)
{
    LOG("entry");

    if (_audio)
    {
        delete_fluid_audio_driver(_audio);
    }

    if (_synth)
    {
        delete_fluid_synth(_synth);
    }

    if (_settings)
    {
        delete_fluid_settings(_settings);
    }

    beepemu_stop();
    if (NULL != _beep.ops)
    {
        snd_device_close(&_beep);
    }

    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

static bool ddcall
fluid_write(snd_device *dev, const midi_event *event)
{
    uint8_t     status = event->status;
    const char *msg = event->msg;

    int channel = status & 0xF;
    if (0xF0 != (status & 0xF0))
    {
        status &= 0xF0;
    }

    switch (status)
    {
    case MIDI_MSG_NOTEOFF:
    case MIDI_MSG_NOTEON:
    case MIDI_MSG_POLYPRESS: {
        int key = msg[0];
        int velocity = msg[1];

        LOG("%3d %-10.10s %3d %3d", channel,
            (MIDI_MSG_NOTEOFF == status)
                ? "Note Off"
                : ((MIDI_MSG_NOTEON == status) ? "Note On" : "PolyPress."),
            key, velocity);
        if (_synth)
        {
            if (MIDI_MSG_NOTEOFF == status)
            {
                fluid_synth_noteoff(_synth, channel, key);
            }
            else if (MIDI_MSG_NOTEON == status)
            {
                fluid_synth_noteon(_synth, channel, key, velocity);
            }
            else
            {
                fluid_synth_key_pressure(_synth, channel, key, velocity);
            }
        }
        break;
    }

    case MIDI_MSG_CONTROL: {
        int control = msg[0];
        int value = msg[1];

        char control_str[32] = "";
        switch (control)
        {
        case 122:
            sprintf(control_str, "(Local Control %s)", value ? "On" : "Off");
            break;
        case 123:
            if (0 == value)
            {
                sprintf(control_str, "(All Notes Off)");
            }
            break;
        case 124:
            if (0 == value)
            {
                sprintf(control_str, "(Omni Mode Off)");
            }
            break;
        case 125:
            if (0 == value)
            {
                sprintf(control_str, "(Omni Mode On)");
            }
            break;
        case 126:
            sprintf(control_str, "(Mono Mode On)");
            break;
        case 127:
            if (0 == value)
            {
                sprintf(control_str, "(Mono Mode Off)");
            }
            break;
        }

        LOG("%3d %-10.10s %3d %3d %s", channel, "ControlCh.", control, value,
            control_str);
        if (_synth)
        {
            fluid_synth_cc(_synth, channel, control, value);
        }
        break;
    }

    case MIDI_MSG_PROGRAM: {
        int program = msg[0];

        LOG("%3d %-10.10s %3d", channel, "ProgramCh.", program);
        if (_synth)
        {
            fluid_synth_program_change(_synth, channel, program);
        }
        break;
    }

    case MIDI_MSG_CHANPRESS: {
        int value = msg[0];

        LOG("%3d %-10.10s %3d", channel, "ChanPress.", value);
        if (_synth)
        {
            fluid_synth_channel_pressure(_synth, channel, value);
        }
        break;
    }

    case MIDI_MSG_PITCHWHEEL: {
        int value_low = msg[0];
        int value_high = msg[1];
        int pitch_bend = (int)((value_high << 7) | value_low);

        LOG("%3d %-10.10s %+6d", channel, "PitchWheel", 0x2000 - pitch_bend);
        if (_synth)
        {
            fluid_synth_pitch_bend(_synth, channel, pitch_bend);
        }
        break;
    }

    case MIDI_MSG_SYSEX: {
        int mfg = msg[0];
        int mfg_len = 1;
        if (0 != mfg)
        {
            LOG("COM %-10.10s %02X", "SystemEx.", mfg);
        }
        else
        {
            mfg_len = 3;
            int mfg2 = msg[1];
            int mfg3 = msg[2];
            LOG("COM %-10.10s %06X", "SystemEx.",
                (mfg << 14) | (mfg2 << 7) | mfg3);
        }

        char hexdump[16 * 3 + 1] = "";
        for (int i = 0; i < event->msg_length - mfg_len; i++)
        {
            sprintf(hexdump + 3 * (i % 16), "%02X ", msg[mfg_len + i] & 0xFF);
            if ((15 == (i % 16)) || ((event->msg_length - mfg_len - 1) == i))
            {
                LOG("    %s", hexdump);
            }
        }

        if (_synth)
        {
            fluid_synth_sysex(_synth, msg, event->msg_length, NULL, NULL, NULL,
                              false);
        }
        break;
    }

    case MIDI_MSG_POSITION: {
        int value_low = msg[0];
        int value_high = msg[1];

        LOG("COM %-10.10s %+6d", "SongPos.", (value_high << 7) | value_low);
        break;
    }

    case MIDI_MSG_SONG: {
        int song = msg[0];

        LOG("COM %-10.10s %3d", "SongSelect", song);
        break;
    }

    case MIDI_MSG_TUNE: {
        LOG("COM Tune");
        break;
    }

    case MIDI_MSG_CLOCK: {
        LOG("COM Clock");
        break;
    }

    case MIDI_MSG_START: {
        LOG("COM Start");
        break;
    }

    case MIDI_MSG_CONTINUE: {
        LOG("COM Continue");
        break;
    }

    case MIDI_MSG_STOP: {
        LOG("COM Stop");
        break;
    }

    case MIDI_MSG_SENSING: {
        LOG("COM ActSensing");
        break;
    }

    case MIDI_MSG_META: {
        uint8_t type = msg[0];
        LOG("COM %-10.10s %3d", "MetaEvent", type);

        uint32_t    data_len;
        const char *data = msg + 1 + midi_read_vlq(msg + 1, &data_len);

        if ((MIDI_META_TEXT <= type) && (MIDI_META_CUEPOINT >= type))
        {
            LOG("    '%.*s'", data_len, data);
        }
        else
        {
            char hexdump[16 * 3 + 1] = "";
            for (int i = 0; i < data_len; i++)
            {
                sprintf(hexdump + 3 * (i % 16), "%02X ", data[i] & 0xFF);
                if ((15 == (i % 16)) || ((data_len - 1) == i))
                {
                    LOG("    %s", hexdump);
                }
            }
        }
        break;
    }
    }

    if (NULL != _beep.ops)
    {
        snd_device_write(&_beep, event);
    }

    return true;
}

static bool ddcall
fluid_tick(snd_device *dev, uint32_t ts)
{
    return snd_device_tick(&_beep, ts);
}

static snd_device_ops _ops = {fluid_open, fluid_close, fluid_write, fluid_tick};

int
__fluid_init(void)
{
    snd_device dev = {"fluid", "FluidSynth", &_ops, NULL};
    return snd_register_device(&dev);
}
