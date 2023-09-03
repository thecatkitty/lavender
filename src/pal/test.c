#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <fluidsynth.h>

#include <fmt/exe.h>
#include <fmt/wave.h>
#include <fmt/zip.h>
#include <gfx.h>
#include <pal.h>
#include <snd.h>

#include "pal_impl.h"

typedef struct
{
    long size;
} fdata;

extern char _binary_obj_version_txt_start[];

static void
_flush_wav(void);

// FIXME: W/A for https://sourceware.org/bugzilla/show_bug.cgi?id=30719
#ifndef __x86_64__
extern
#endif
    char __w32_rsrc_start[]
#ifdef __x86_64__
    __attribute__((section(".rsrc"))) = {}
#endif
;

static long _start_msec;

static SDL_Keycode    _keycode;
static bool           _mouse_enabled = false;
static uint16_t       _mouse_x, _mouse_y, _mouse_buttons;
static gfx_dimensions _mouse_cell;

static const char *SOUNDFONTS[] = {"/usr/share/sounds/sf2/default-GM.sf2",
                                   "/usr/share/soundfonts/default-GM.sf2",
                                   "/usr/share/sounds/sf2/default.sf2",
                                   "/usr/share/soundfonts/default.sf2", NULL};

static fluid_settings_t     *_fluid_settings;
static fluid_synth_t        *_fluid_synth;
static fluid_audio_driver_t *_fluid_audio;

#define PCSPK_CYCLE 65536
#define PCSPK_RATE  44100

uint16_t _pcspk_div = 0;
uint32_t _pcspk_counter = 0;
bool     _pcspk_enabled = false;
uint32_t _pcspk_last = 0;

FILE           *_wav = NULL;
iff_head        _wav_chkroot;
iff_head        _wav_chkfmt;
iff_head        _wav_chkdata;
long            _wav_chkdata_pos;
wave_pcm_format _wav_pcmfmt;

void
pal_initialize(int argc, char *argv[])
{
    struct timespec time;
    clock_gettime(CLOCK_BOOTTIME, &time);
    _start_msec = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    LOG("entry");

    if (!zip_open(argv[0]))
    {
        LOG("cannot open the archive '%s'. %s", argv[0], strerror(errno));
        abort();
    }

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        __pal_assets[i].inzip = -1;
        __pal_assets[i].flags = 0;
        __pal_assets[i].data = NULL;
    }

    if (0 > SDL_Init(0))
    {
        LOG("cannot initialize SDL. %s", SDL_GetError());
        abort();
    }

    if (!gfx_initialize())
    {
        LOG("cannot initialize graphics");
        pal_cleanup();
    }

    gfx_get_glyph_dimensions(&_mouse_cell);

    if (0 > SDL_InitSubSystem(SDL_INIT_AUDIO))
    {
        LOG("cannot initialize audio subsystem. %s", SDL_GetError());
        goto end;
    }

    _fluid_settings = new_fluid_settings();
    if (NULL == _fluid_settings)
    {
        LOG("cannot create FluidSynth settings");
        goto end;
    }

    fluid_settings_setstr(_fluid_settings, "audio.driver", "sdl2");

    _fluid_synth = new_fluid_synth(_fluid_settings);
    if (NULL == _fluid_synth)
    {
        LOG("cannot create the synthesizer");
        goto end;
    }

    int sfont = FLUID_FAILED;
    for (const char **path = SOUNDFONTS; *path; path++)
    {
        LOG("trying SoundFont '%s'", *path);
        if (FLUID_FAILED !=
            (sfont = fluid_synth_sfload(_fluid_synth, *path, 1)))
        {
            break;
        }
    }

    if (FLUID_FAILED == sfont)
    {
        LOG("cannot load any SoundFont");
        delete_fluid_synth(_fluid_synth);
        _fluid_synth = NULL;
        goto end;
    }

    _fluid_audio = new_fluid_audio_driver(_fluid_settings, _fluid_synth);
    if (NULL == _fluid_audio)
    {
        LOG("cannot create the audio driver");
        delete_fluid_synth(_fluid_synth);
        _fluid_synth = NULL;
        goto end;
    }

end:
}

void
pal_cleanup(void)
{
    LOG("entry");

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        if (NULL != __pal_assets[i].data)
        {
            zip_free_data(__pal_assets[i].data);
        }
    }

    if (_fluid_audio)
    {
        delete_fluid_audio_driver(_fluid_audio);
    }

    if (_fluid_synth)
    {
        delete_fluid_synth(_fluid_synth);
    }

    if (_fluid_settings)
    {
        delete_fluid_settings(_fluid_settings);
    }

    gfx_cleanup();

    SDL_Quit();

    if (_wav)
    {
        _flush_wav();
        fclose(_wav);
    }
}

void
pal_handle(void)
{
    SDL_PumpEvents();

    SDL_Event e;
    if (1 > SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    {
        return;
    }

    switch (e.type)
    {
    case SDL_KEYDOWN: {
        LOG("key '%s' down", SDL_GetKeyName(e.key.keysym.sym));
        _keycode = e.key.keysym.sym;
        break;
    }

    case SDL_KEYUP:
        LOG("key '%s' up", SDL_GetKeyName(e.key.keysym.sym));
        _keycode = 0;
        break;

    case SDL_MOUSEMOTION: {
        if (!_mouse_enabled)
        {
            break;
        }

        LOG("mouse x: %d, y: %d", e.motion.x, e.motion.y);
        _mouse_x = e.motion.x / _mouse_cell.width;
        _mouse_y = e.motion.y / 16;
        break;
    }

    case SDL_MOUSEBUTTONDOWN: {
        if (!_mouse_enabled)
        {
            break;
        }

        LOG("mouse button %u down", e.button.button);
        if (SDL_BUTTON_LEFT == e.button.button)
        {
            _mouse_buttons |= PAL_MOUSE_LBUTTON;
        }
        else if (SDL_BUTTON_RIGHT == e.button.button)
        {
            _mouse_buttons |= PAL_MOUSE_RBUTTON;
        }
        break;
    }

    case SDL_MOUSEBUTTONUP: {
        if (!_mouse_enabled)
        {
            break;
        }

        LOG("mouse button %u up", e.button.button);
        if (SDL_BUTTON_LEFT == e.button.button)
        {
            _mouse_buttons &= ~PAL_MOUSE_LBUTTON;
        }
        else if (SDL_BUTTON_RIGHT == e.button.button)
        {
            _mouse_buttons &= ~PAL_MOUSE_RBUTTON;
        }
        break;
    }
    }
}

uint32_t
pal_get_counter(void)
{
    struct timespec time;
    clock_gettime(CLOCK_BOOTTIME, &time);

    long time_msec = time.tv_sec * 1000 + time.tv_nsec / 1000000;
    return time_msec - _start_msec;
}

uint32_t
pal_get_ticks(unsigned ms)
{
    return ms;
}

void
pal_sleep(unsigned ms)
{
    LOG("entry, ms: %u", ms);

    usleep(ms);
}

const char *
pal_get_version_string(void)
{
    return _binary_obj_version_txt_start;
}

uint32_t
pal_get_medium_id(const char *tag)
{
    LOG("entry");
    return 0;
}

uint16_t
pal_get_keystroke(void)
{
    if (!_keycode)
    {
        return 0;
    }

    int c = _keycode;
    _keycode = 0;
    if (255 < c)
    {
        c = 0;
    }

    if (islower(c))
    {
        c = toupper(c);
    }

    LOG("keystroke: %#.2x", c);
    return c;
}

void
pal_enable_mouse(void)
{
    LOG("entry");

    _mouse_enabled = true;
    return;
}

void
pal_disable_mouse(void)
{
    LOG("entry");

    _mouse_enabled = false;
    return;
}

uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y)
{
    if (!_mouse_enabled)
    {
        return 0;
    }

    *x = _mouse_x;
    *y = _mouse_y;
    if (_mouse_buttons)
    {
        LOG("x: %u, y: %u, buttons: %#x", *x, *y, _mouse_buttons);
    }
    return _mouse_buttons;
}

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    int length = exe_pe_load_string(__w32_rsrc_start, id, buffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);

        LOG("exit, string missing");
        return sizeof(msg) - 1;
    }

    LOG("exit, '%s'", buffer);
    return length;
}

extern void
sndd_send(midi_event *event);

void
snd_send(midi_event *event)
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
        if (_fluid_synth)
        {
            if (MIDI_MSG_NOTEOFF == status)
            {
                fluid_synth_noteoff(_fluid_synth, channel, key);
            }
            else if (MIDI_MSG_NOTEON == status)
            {
                fluid_synth_noteon(_fluid_synth, channel, key, velocity);
            }
            else
            {
                fluid_synth_key_pressure(_fluid_synth, channel, key, velocity);
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
        if (_fluid_synth)
        {
            fluid_synth_cc(_fluid_synth, channel, control, value);
        }
        break;
    }

    case MIDI_MSG_PROGRAM: {
        int program = msg[0];

        LOG("%3d %-10.10s %3d", channel, "ProgramCh.", program);
        if (_fluid_synth)
        {
            fluid_synth_program_change(_fluid_synth, channel, program);
        }
        break;
    }

    case MIDI_MSG_CHANPRESS: {
        int value = msg[0];

        LOG("%3d %-10.10s %3d", channel, "ChanPress.", value);
        if (_fluid_synth)
        {
            fluid_synth_channel_pressure(_fluid_synth, channel, value);
        }
        break;
    }

    case MIDI_MSG_PITCHWHEEL: {
        int value_low = msg[0];
        int value_high = msg[1];
        int pitch_bend = (int)((value_high << 7) | value_low);

        LOG("%3d %-10.10s %+6d", channel, "PitchWheel", 0x2000 - pitch_bend);
        if (_fluid_synth)
        {
            fluid_synth_pitch_bend(_fluid_synth, channel, pitch_bend);
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

        if (_fluid_synth)
        {
            fluid_synth_sysex(_fluid_synth, msg, event->msg_length, NULL, NULL,
                              NULL, false);
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

    sndd_send(event);
}

void
_flush_wav(void)
{
    if (!_wav)
    {
        _wav_chkroot.type.dw = IFF_FOURCC_RIFF.dw;

        _wav_chkfmt.type.dw = WAVE_FOURCC_FMT.dw;
        _wav_pcmfmt.wf.format = WAVE_FORMAT_PCM;
        _wav_pcmfmt.wf.channels = 1;
        _wav_pcmfmt.wf.sample_rate = PCSPK_RATE;
        _wav_pcmfmt.wf.byte_rate = PCSPK_RATE;
        _wav_pcmfmt.wf.block_align = _wav_pcmfmt.wf.byte_rate;
        _wav_pcmfmt.bits_per_sample = 8;
        _wav_chkfmt.length = sizeof(_wav_pcmfmt);

        _wav_chkdata.type.dw = WAVE_FOURCC_DATA.dw;
        _wav_chkdata.length = 0;

        _wav_chkroot.length = sizeof(WAVE_FOURCC_WAVE) + sizeof(_wav_chkfmt) +
                              _wav_chkfmt.length + sizeof(_wav_chkdata) +
                              _wav_chkdata.length;

        _wav = fopen("speaker.wav", "wb");
        fwrite(&_wav_chkroot, sizeof(_wav_chkroot), 1, _wav);
        fwrite(&WAVE_FOURCC_WAVE, sizeof(WAVE_FOURCC_WAVE), 1, _wav);

        fwrite(&_wav_chkfmt, sizeof(_wav_chkfmt), 1, _wav);
        fwrite(&_wav_pcmfmt, _wav_chkfmt.length, 1, _wav);

        _wav_chkdata_pos = ftell(_wav);
        fwrite(&_wav_chkdata, sizeof(_wav_chkdata), 1, _wav);
        _wav = freopen(NULL, "rb+", _wav);
    }

    fseek(_wav, 0, SEEK_END);

    uint32_t now = pal_get_counter();
    uint32_t length = (now - _pcspk_last) * PCSPK_RATE / 1000;
    uint32_t cycle = PCSPK_RATE * _pcspk_div / 1193182;
    if (_pcspk_enabled)
    {
        for (int i = 0; i < length; i++, _pcspk_counter++)
        {
            fputc(128 + ((_pcspk_counter % cycle) < (cycle / 2) ? -96 : +96),
                  _wav);
        }
    }
    else
    {
        for (int i = 0; i < length; i++, _pcspk_counter++)
        {
            fputc(128, _wav);
        }
    }

    _wav_chkroot.length += length;
    fseek(_wav, 0, SEEK_SET);
    fwrite(&_wav_chkroot, sizeof(_wav_chkroot), 1, _wav);

    _wav_chkdata.length += length;
    fseek(_wav, _wav_chkdata_pos, SEEK_SET);
    fwrite(&_wav_chkdata, sizeof(_wav_chkdata), 1, _wav);

    fseek(_wav, 0, SEEK_END);
    _pcspk_last = now;
}

void
dospc_beep(uint16_t divisor)
{
    _flush_wav();
    _pcspk_div = divisor;
    _pcspk_enabled = true;
}

void
dospc_silence(void)
{
    _flush_wav();
    _pcspk_enabled = false;
}
