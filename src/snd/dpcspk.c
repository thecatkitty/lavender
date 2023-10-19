#include <pal.h>
#include <platform/dospc.h>
#include <snd.h>

#define KEY_MIN          14 // PIT incapable of frequencies lower than D0
#define KEY_A4           69
#define FREQ_A4          440
#define INPUT_FREQ_U32F8 305454507UL

// As 8-bit fractional part; integer part always 1
static const uint8_t KEY_MULTIPLIERS[] = {
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

static bool
pcspk_open(void)
{
    return true;
}

static void
pcspk_close(void)
{
}

static bool
pcspk_write(const midi_event *event)
{
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

    if (0 != channel)
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

        if ((MIDI_MSG_NOTEOFF == status) || (0 == velocity))
        {
            dospc_silence();
            return true;
        }

        if (KEY_MIN > key)
        {
            return false;
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
        uint16_t divisor = INPUT_FREQ_U32F8 / freq;
        dospc_beep(divisor);
        break;
    }
    }

    return true;
}

snd_device_protocol __snd_dpcspk = {pcspk_open, pcspk_close, pcspk_write};
