#ifndef _FMT_MIDI_H_
#define _FMT_MIDI_H_

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

#endif // _FMT_MIDI_H_
