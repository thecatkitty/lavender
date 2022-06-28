#ifndef _FMT_SPK_H_
#define _FMT_SPK_H_

#include <base.h>

#pragma pack(push, 1)
typedef struct
{
    uint8_t format;
    uint8_t ticks_per_second;
} spk_header;

typedef struct
{
    uint8_t  duration;
    uint16_t divisor;
} spk_note3;
#pragma pack(pop)

#define SPK_FORMAT_NOTE3 3

#define SPK_NOTE_DURATION_STOP 0

#endif // _FMT_SPK_H_
