#ifndef _FMT_BMP_
#define _FMT_BMP_

#include <gfx.h>
#include <pal.h>

#pragma pack(push, 1)
typedef struct
{
    uint16_t type;
    uint32_t size;
    uint16_t _reserved1;
    uint16_t _reserved2;
    uint32_t off_bits;
} bmp_file_header;

typedef struct
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t  horizontal_pxpm;
    int32_t  vertical_pxpm;
    uint32_t colors_used;
    uint32_t colors_important;
} bmp_info_header;

typedef struct
{
    bmp_info_header info;

    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_msk;
    uint32_t cs_type;
    int32_t  endpoints[3][3];
    uint32_t gamma_red;
    uint32_t gamma_green;
    uint32_t gamma_blue;
    uint32_t intent;
    uint32_t profile_data;
    uint32_t profile_size;
    uint32_t _reserved;
} bmp_v5_header;
#pragma pack(pop)

#define BMP_MAGIC 0x4D42

enum
{
    BMP_COMPRESSION_RGB = 0x0000,
    BMP_COMPRESSION_RLE8 = 0x0001,
    BMP_COMPRESSION_RLE4 = 0x0002,
    BMP_COMPRESSION_BITFIELDS = 0x0003,
    BMP_COMPRESSION_JPEG = 0x0004,
    BMP_COMPRESSION_PNG = 0x0005,
    BMP_COMPRESSION_CMYK = 0x000B,
    BMP_COMPRESSION_CMYKRLE8 = 0x000C,
    BMP_COMPRESSION_CMYKRLE4 = 0x000D
};

extern bool
bmp_load_bitmap(gfx_bitmap *bm, hasset asset);

extern void
bmp_dispose_bitmap(gfx_bitmap *bm);

#endif
