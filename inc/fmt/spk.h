#ifndef _FMT_SPK_H_
#define _FMT_SPK_H_

#include <base.h>

#pragma pack(push, 1)
typedef struct
{
    uint8_t Format;
    uint8_t TicksPerSecond;
} SPK_HEADER;

typedef struct
{
    uint8_t  Duration;
    uint16_t Divisor;
} SPK_NOTE3;
#pragma pack(pop)

#define SPK_FORMAT_NOTE3 3

#define SPK_NOTE_DURATION_STOP 0

#endif // _FMT_SPK_H_
