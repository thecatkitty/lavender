#ifndef _SND_SEQ_H_
#define _SND_SEQ_H_

#include <base.h>

// Open MIDI file for playback
extern bool
sndseq_open(const char *name);

// Close MIDI file
extern bool
sndseq_close(void);

// Start MIDI sequence
extern bool
sndseq_start(void);

// Step MIDI sequence forward
extern bool
sndseq_step(void);

#endif // _SND_SEQ_H_
