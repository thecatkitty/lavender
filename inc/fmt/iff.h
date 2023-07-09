#ifndef _FMT_IFF_H_
#define _FMT_IFF_H_

#include <base.h>

typedef union {
    char     c[4];
    uint32_t dw;
} iff_fourcc;

typedef struct
{
    iff_fourcc type;
    uint32_t   length;
} iff_head;

typedef enum
{
    IFF_MODE_BE = 0x0001,
    IFF_MODE_NOHEAD = 0x0002,
} iff_mode;

typedef struct
{
    const char *data;
    iff_mode    mode;
    uint32_t    length;
} iff_context;
typedef struct
{
    iff_fourcc  type;
    uint32_t    length;
    const char *data;
} iff_chunk;

#define IFF_FOURCC(fourcc)                                                     \
    {                                                                          \
        .c = fourcc                                                            \
    }

static const iff_fourcc IFF_FOURCC_RIFF = IFF_FOURCC("RIFF");
static const iff_fourcc IFF_FOURCC_FORM = IFF_FOURCC("FORM");
static const iff_fourcc IFF_FOURCC_LIST = IFF_FOURCC("LIST");
static const iff_fourcc IFF_FOURCC_CAT = IFF_FOURCC("CAT ");

extern iff_context *
iff_open(void *data, uint16_t length);

extern bool
iff_close(iff_context *ctx);

extern bool
iff_next_chunk(iff_context *ctx, iff_chunk *it);

#endif // _FMT_IFF_H_
