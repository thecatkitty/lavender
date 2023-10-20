#include <conio.h>
#include <stdio.h>

#include <pal.h>
#include <snd.h>

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
#define OPL2_REG_DEPTH_RYTHM         0xBD
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

static void
_write(uint8_t addr, uint8_t value)
{
    outp(OPL2_ADDRESS, addr);
    outp(OPL2_DATA, value);
}

static void
_reset(void)
{
    for (uint8_t addr = 0x01; addr <= 0xF5; addr++)
    {
        _write(addr, 0);
    }
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
    return false;
}

snd_device_protocol __snd_dopl2 = {opl2_open, opl2_close, opl2_write, "opl2",
                                   "Yamaha YM3812 (OPL2)"};
