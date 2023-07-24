#ifndef _FMT_MIDI_H_
#define _FMT_MIDI_H_

#include <fmt/iff.h>

typedef struct
{
    uint16_t format;
    uint16_t ntrks;
    uint16_t division;
} midi_mthd;

typedef struct
{
    uint32_t    delta;
    const char *msg;
    size_t      msg_length;
} midi_event;

static const iff_fourcc MIDI_FOURCC_MTHD = IFF_FOURCC("MThd");
static const iff_fourcc MIDI_FOURCC_MTRK = IFF_FOURCC("MTrk");

#define MIDI_MTHD_DIVISION_SMPTE_FLAG   0x8000
#define MIDI_MTHD_DIVISION_SMPTE_OFFSET 8
#define MIDI_MTHD_DIVISION_SMPTE_MASK   0xFF
#define MIDI_MTHD_DIVISION_TPF_OFFSET   0
#define MIDI_MTHD_DIVISION_TPF_MASK     0xFF

#define MIDI_MSG_NOTEOFF    0x80
#define MIDI_MSG_NOTEON     0x90
#define MIDI_MSG_POLYPRESS  0xA0
#define MIDI_MSG_CONTROL    0xB0
#define MIDI_MSG_PROGRAM    0xC0
#define MIDI_MSG_CHANPRESS  0xD0
#define MIDI_MSG_PITCHWHEEL 0xE0

#define MIDI_MSG_SYSEX    0xF0
#define MIDI_MSG_POSITION 0xF2
#define MIDI_MSG_SONG     0xF3
#define MIDI_MSG_TUNE     0xF6
#define MIDI_MSG_ENDEX    0xF7

#define MIDI_MSG_CLOCK    0xF8
#define MIDI_MSG_START    0xFA
#define MIDI_MSG_CONTINUE 0xFB
#define MIDI_MSG_STOP     0xFC
#define MIDI_MSG_SENSING  0xFE
#define MIDI_MSG_META     0xFF

#define MIDI_MAX_KEY      0x7F
#define MIDI_MAX_VELOCITY 0x7F

extern size_t
midi_read_event(const char *buffer, midi_event *event);

extern size_t
midi_read_vlq(const char *buffer, uint32_t *vlq);

#endif // _FMT_MIDI_H_
