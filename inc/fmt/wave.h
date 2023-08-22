#ifndef _FMT_WAVE_H_
#define _FMT_WAVE_H_

#include <fmt/iff.h>

#pragma pack(push, 1)
typedef struct
{
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
} wave_format;

typedef struct
{
    wave_format wf;
    uint16_t    bits_per_sample;
} wave_pcm_format;
#pragma pack(pop)

#define WAVE_FORMAT_PCM 0x0001

static const iff_fourcc WAVE_FOURCC_WAVE = IFF_FOURCC("WAVE");
static const iff_fourcc WAVE_FOURCC_FMT = IFF_FOURCC("fmt ");
static const iff_fourcc WAVE_FOURCC_DATA = IFF_FOURCC("data");

#endif // _FMT_WAVE_H_
