#include <conio.h>
#include <stdio.h>

#include <pal.h>
#include <snd.h>

#include "opl2.h"

// Standard OPL2 I/O space base address
#define OPL2_IO_BASE 0x388

// OPL2 I/O ports
#define OPL2_ADDRESS     (OPL2_IO_BASE + 0)
#define OPL2_DATA        (OPL2_IO_BASE + 1)
#define OPL2_STATUS      OPL2_ADDRESS
#define OPL2_IRQ         (1 << 7)
#define OPL2_FLAG_TIMER1 (1 << 6)
#define OPL2_FLAG_TIMER2 (1 << 5)

// OPL2 internal registers
#define OPL2_REG_TEST                0x01
#define OPL2_WAVE_SELECT             (1 << 5)
#define OPL2_REG_TIMER1              0x02
#define OPL2_REG_TIMER2              0x03
#define OPL2_REG_TIMER_CONTROL       0x04
#define OPL2_IRQ_RESET               (1 << 7)
#define OPL2_MASK_TIMER1             (1 << 6)
#define OPL2_MASK_TIMER2             (1 << 5)
#define OPL2_START_TIMER2            (1 << 1)
#define OPL2_START_TIMER1            (1 << 0)
#define OPL2_REG_CSM_NOTESEL         0x08
#define OPL2_COMPOSITE_SINE          (1 << 7)
#define OPL2_NOTE_SELECT             (1 << 6)
#define OPL2_REG_AM_VIBx(x)          (0x20 + x)
#define OPL2_TREMOLO                 (1 << 7)
#define OPL2_VIBRATO                 (1 << 6)
#define OPL2_SUSTAIN                 (1 << 5)
#define OPL2_KSR                     (1 << 4)
#define OPL2_MULTIPLIER_MASK         (0xF << 0)
#define OPL2_REG_KSL_TLx(x)          (0x40 + x)
#define OPL2_KEY_SCALING_MASK        (0x3 << 6)
#define OPL2_TOTAL_LEVEL_MASK        (0x3F << 0)
#define OPL2_REG_ATTACK_DECAYx(x)    (0x60 + x)
#define OPL2_ATTACK_MASK             (0xF << 4)
#define OPL2_DECAY_MASK              (0xF << 0)
#define OPL2_REG_SUSTAIN_RELEASEx(x) (0x80 + x)
#define OPL2_SUSTAIN_MASK            (0xF << 4)
#define OPL2_RELEASE_MASK            (0xF << 0)
#define OPL2_REG_FNUMLOx(x)          (0xA0 + x)
#define OPL2_REG_KEYON_FNUMHIx(x)    (0xB0 + x)
#define OPL2_KEYON                   (1 << 5)
#define OPL2_BLOCK_MASK              (0x7 << 2)
#define OPL2_FNUMHI_MASK             (0x3 << 0)
#define OPL2_REG_DEPTH_RHYTHM        0xBD
#define OPL2_TREMOLO_DEPTH           (1 << 7)
#define OPL2_VIBRATO_DEPTH           (1 << 6)
#define OPL2_RHYTHM                  (1 << 5)
#define OPL2_BASSDRUM                (1 << 4)
#define OPL2_SNAREDRUM               (1 << 3)
#define OPL2_TOMTOM                  (1 << 2)
#define OPL2_CYMBAL                  (1 << 1)
#define OPL2_HIHAT                   (1 << 0)
#define OPL2_REG_FEEDBACK_CONNx(x)   (0xC0 + x)
#define OPL2_FEEDBACK_MASK           (0x7 << 1)
#define OPL2_CONNECTION              (1 << 0)
#define OPL2_REG_WAVE_SELECTx(x)     (0xE0 + x)

#define VOICES 6

#define MODULATOR 0
#define CARRIER   1

// Offsets within AM_VIBx, KSL_TLx, ATTACK_DECAYx, SUSTAIN_RELEASEx,
// and WAVE_SELECTx register blocks, for both operators of six supported voices
static const uint8_t OPL2_VOICE_OFFSET[2][VOICES] = {
    [MODULATOR] = {0, 1, 2, 8, 9, 10}, [CARRIER] = {3, 4, 5, 11, 12, 13}};

// MIDI note to FNUM mapping
static const uint16_t OPL2_MIDI_FNUM[2 * 12] = {
    // Lowest octave
    345, 365, 387, 410, 435, 460, 488, 517, 547, 580, 615, 651,
    // Every other octave
    690, 731, 774, 820, 869, 921, 975, 517, 547, 580, 615, 651};

// Current voice mapping
static uint8_t _voices[VOICES];

static void
_write(uint8_t addr, uint8_t value)
{
    outp(OPL2_ADDRESS, addr);
    for (int i = 0; i < 6; i++)
    {
        inp(OPL2_ADDRESS);
    }

    outp(OPL2_DATA, value);
    for (int i = 0; i < 35; i++)
    {
        inp(OPL2_ADDRESS);
    }
}

static void
_reset(void)
{
    for (uint8_t addr = 0x01; addr <= 0xF5; addr++)
    {
        _write(addr, 0);
    }
}

static void
_set_instrument(uint8_t voice, uint8_t instrument)
{
    _voices[voice] = instrument;

    uint8_t mod_offset = OPL2_VOICE_OFFSET[MODULATOR][voice];
    uint8_t car_offset = OPL2_VOICE_OFFSET[CARRIER][voice];

    const opl2_patch *patch = __snd_gm_opl2 + instrument;
    _write(OPL2_REG_KSL_TLx(car_offset),
           patch->car_level | OPL2_TOTAL_LEVEL_MASK);
    _write(OPL2_REG_WAVE_SELECTx(car_offset), patch->car_wave_select);
    _write(OPL2_REG_SUSTAIN_RELEASEx(car_offset),
           (patch->car_sustain << 4) | patch->car_release);
    _write(OPL2_REG_ATTACK_DECAYx(car_offset),
           (patch->car_attack << 4) | patch->car_decay);
    _write(OPL2_REG_AM_VIBx(car_offset), patch->car_amvib_reg);

    _write(OPL2_REG_KSL_TLx(mod_offset), patch->mod_level);
    _write(OPL2_REG_WAVE_SELECTx(mod_offset), patch->mod_wave_select);
    _write(OPL2_REG_SUSTAIN_RELEASEx(mod_offset),
           (patch->mod_sustain << 4) | patch->mod_release);
    _write(OPL2_REG_ATTACK_DECAYx(mod_offset),
           (patch->mod_attack << 4) | patch->mod_decay);
    _write(OPL2_REG_AM_VIBx(mod_offset), patch->mod_amvib_reg);

    _write(OPL2_REG_FEEDBACK_CONNx(voice), patch->feedback_conn_reg);
}

static uint16_t
_get_block_fnum(uint8_t key)
{
    uint16_t fnum = OPL2_MIDI_FNUM[(12 > key) ? key : (12 + (key % 12))];

    uint8_t block = 0;
    if (19 < key)
    {
        block = (key - 19) / 12;
    }

    if (7 < block)
    {
        block = 7;
    }

    return ((uint16_t)block << 10) | fnum;
}

static uint8_t
_get_level(uint8_t volume, uint8_t ksl_tl_reg)
{
    unsigned max_level =
        OPL2_TOTAL_LEVEL_MASK - (ksl_tl_reg & OPL2_TOTAL_LEVEL_MASK);
    unsigned level = (max_level * volume) / 127;

    return (ksl_tl_reg & OPL2_KEY_SCALING_MASK) |
           ((OPL2_TOTAL_LEVEL_MASK - level) & OPL2_TOTAL_LEVEL_MASK);
}

static bool
opl2_open(void)
{
    // Reset timers and interrupts
    _write(OPL2_REG_TIMER_CONTROL, OPL2_MASK_TIMER1 | OPL2_MASK_TIMER2);
    _write(OPL2_REG_TIMER_CONTROL, OPL2_IRQ_RESET);

    // Read status register and store the result - should be 0x00
    uint8_t status_a = inp(OPL2_STATUS) & 0xE0;

    // Start timer and wait
    _write(OPL2_REG_TIMER1, 0xFF);
    _write(OPL2_REG_TIMER_CONTROL, OPL2_MASK_TIMER2 | OPL2_START_TIMER1);
    pal_sleep(1);

    // Read status register and store the result - should be 0xC0
    uint8_t status_b = inp(OPL2_STATUS) & 0xE0;

    // Reset timers and interrupts
    _write(OPL2_REG_TIMER_CONTROL, OPL2_MASK_TIMER1 | OPL2_MASK_TIMER2);
    _write(OPL2_REG_TIMER_CONTROL, OPL2_IRQ_RESET);

    if (status_a || ((OPL2_IRQ | OPL2_FLAG_TIMER1) != status_b))
    {
        return false;
    }

    _reset();
    _write(OPL2_REG_TEST, OPL2_WAVE_SELECT);
    _write(OPL2_REG_TIMER_CONTROL, 0);
    _write(OPL2_REG_CSM_NOTESEL, OPL2_NOTE_SELECT);
    _write(OPL2_REG_DEPTH_RHYTHM, 0);

    for (uint8_t voice = 0; voice < VOICES; voice++)
    {
        _set_instrument(voice, 0);
    }

    return true;
}

static void
opl2_close(void)
{
    _reset();
}

static bool
opl2_write(const midi_event *event)
{
    uint8_t     status = event->status;
    const char *msg = event->msg;

    uint8_t channel = status & 0xF;
    if (0xF0 != (status & 0xF0))
    {
        status &= 0xF0;
    }

    if (VOICES <= channel)
    {
        return true;
    }

    switch (status)
    {
    case MIDI_MSG_NOTEOFF:
    case MIDI_MSG_NOTEON: {
        uint8_t key = msg[0];
        uint8_t velocity = msg[1];

        if ((MIDI_MSG_NOTEOFF == status) || (0 == velocity))
        {
            _write(OPL2_REG_KEYON_FNUMHIx(channel), 0);
            return true;
        }

        uint16_t block_fnum = _get_block_fnum(key);
        _write(OPL2_REG_KSL_TLx(OPL2_VOICE_OFFSET[CARRIER][channel]),
               _get_level(velocity, __snd_gm_opl2[_voices[channel]].car_level));
        _write(OPL2_REG_FNUMLOx(channel), block_fnum & 0xFF);
        _write(OPL2_REG_KEYON_FNUMHIx(channel), OPL2_KEYON | (block_fnum >> 8));
        break;
    }

    case MIDI_MSG_PROGRAM: {
        uint8_t program = msg[0];

        _set_instrument(channel, program);
        break;
    }
    }

    return true;
}

snd_device_protocol __snd_dopl2 = {opl2_open, opl2_close, opl2_write, "opl2",
                                   "Yamaha YM3812 (OPL2)"};
