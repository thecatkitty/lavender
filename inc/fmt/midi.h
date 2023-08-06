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
    uint8_t     status;
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

#define MIDI_META_SEQNUM     0x00
#define MIDI_META_TEXT       0x01
#define MIDI_META_COPYRIGHT  0x02
#define MIDI_META_TRACK      0x03
#define MIDI_META_INSTRUMENT 0x04
#define MIDI_META_LYRIC      0x05
#define MIDI_META_MARKER     0x06
#define MIDI_META_CUEPOINT   0x07
#define MIDI_META_CHANPREFIX 0x20
#define MIDI_META_ENDTRACK   0x2F
#define MIDI_META_TEMPO      0x51
#define MIDI_META_SMPTEO     0x54
#define MIDI_META_TIMESIGN   0x58
#define MIDI_META_KEYSIGN    0x59
#define MIDI_META_SPECIFIC   0x7F

#define MIDI_MAX_KEY      0x7F
#define MIDI_MAX_VELOCITY 0x7F

extern size_t
midi_read_event(const char *buffer, midi_event *event);

extern size_t
midi_read_vlq(const char *buffer, uint32_t *vlq);

#endif // _FMT_MIDI_H_
