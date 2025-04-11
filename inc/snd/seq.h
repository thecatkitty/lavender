#ifndef _SND_SEQ_H_
#define _SND_SEQ_H_

#include <base.h>

// Start MIDI sequence
extern bool
sndseq_start(void *music, uint16_t length);

// Step MIDI sequence forward
extern bool
sndseq_step(void);

#endif // _SND_SEQ_H_
