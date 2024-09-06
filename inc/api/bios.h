#ifndef _API_BIOS_H
#define _API_BIOS_H

#include <assert.h>
#include <i86.h>

#include <base.h>
#include <fmt/edid.h>

#pragma pack(push, 1)
typedef struct
{
    bool     floppy_disk : 1;
    bool     x87 : 1;
    unsigned onboard_memory : 2;
    unsigned initial_video_mode : 2;
    unsigned floppy_drives : 2;
    bool     no_dma : 1;
    unsigned serial_ports : 3;
    bool     game_port : 1;
    bool     serial_printer : 1;
    unsigned parallel_ports : 2;
} bios_equipment;

static_assert(sizeof(uint16_t) == sizeof(bios_equipment),
              "BIOS equipment list size doesn't match specification");

#pragma pack(pop)

static inline uint16_t
bios_get_keystroke(void)
{
    uint16_t ax;
    asm volatile("int $0x16" : "=a"(ax) : "Rah"((uint8_t)0x00));
    return ax;
}

static inline uint16_t
bios_check_keystroke(void)
{
    uint16_t ax;
    asm volatile("int $0x16; jnz end; xor %%ax, %%ax\nend:"
                 : "=a"(ax)
                 : "Rah"((uint8_t)0x01)
                 : "cc");
    return ax;
}

static inline void
bios_set_cursor_position(uint8_t page, uint16_t position)
{
    asm volatile("int $0x10"
                 :
                 : "a"((uint16_t)(0x02 << 8)), "b"((uint16_t)page << 8),
                   "d"(position));
}

static inline void
bios_write_character(uint8_t  page,
                     uint8_t  character,
                     uint8_t  attribute,
                     uint16_t count)
{
    asm volatile("int $0x10"
                 :
                 : "a"((uint16_t)(0x09 << 8) | character),
                   "b"((uint16_t)(page << 8) | attribute), "c"(count));
}

static inline uint8_t
bios_get_video_mode(void)
{
    uint8_t mode;
    asm volatile("int $0x10"
                 : "=a"(mode)
                 : "Rah"((uint8_t)0xF)
                 : "cc", "bx", "cx", "dx");
    return mode;
}

static inline void
bios_set_video_mode(uint8_t mode)
{
    uint16_t ax;
    asm volatile("int $0x10" : "=a"(ax) : "0"(mode) : "cc", "bx", "cx", "dx");
}

static inline short
bios_get_vbedc_capabilities()
{
    unsigned short ax;
    asm volatile("int $0x10" : "=a"(ax) : "a"(0x4F15), "b"(0));
    return ax;
}

static inline short
bios_get_equipment_list(void)
{
    unsigned short ax;
    asm volatile("int $0x11" : "=a"(ax));
    return ax;
}

static inline short
bios_read_edid(far edid_block *edid)
{
    unsigned short ax, es, di;
    es = FP_SEG(edid);
    di = FP_OFF(edid);
    asm volatile("push %%es; mov %%bx, %%es; mov $1, %%bx; int $0x10; pop %%es"
                 : "=a"(ax)
                 : "a"(0x4F15), "b"(es), "c"(0), "d"(0), "D"(di)
                 : "memory");
    return ax;
}

static inline short
bios_reset_disk(uint8_t drive)
{
    unsigned short ax;
    asm volatile("int $0x13"
                 : "=a"(ax)
                 : "a"(0x0000), "d"(drive)
                 : "memory", "cc");
    return (ax >> 8) & 0xFF;
}

static inline short
bios_read_sectors(uint8_t   drive,
                  uint16_t  cylinder,
                  uint8_t   head,
                  uint8_t   sector,
                  uint8_t   count,
                  far char *buffer)
{
    unsigned short ax, bx, cx, dx, es;
    ax = (0x02 << 8) | count;
    cx = (cylinder << 6) | (sector & 0x3F);
    dx = (head << 8) | drive;
    es = FP_SEG(buffer);
    bx = FP_OFF(buffer);
    asm volatile("push %%es; mov %%di, %%es; int $0x13; pop %%es"
                 : "=a"(ax)
                 : "a"(ax), "b"(bx), "c"(cx), "d"(dx), "D"(es)
                 : "memory", "cc");
    return ax;
}

#endif // _API_BIOS_H
