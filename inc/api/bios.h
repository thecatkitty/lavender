#ifndef _API_BIOS_H
#define _API_BIOS_H

#include <assert.h>
#include <i86.h>

#include <base.h>
#include <fmt/edid.h>

#pragma pack(push, 1)
typedef struct
{
    bool     FloppyDisk : 1;
    bool     X87 : 1;
    unsigned OnBoardMemory : 2;
    unsigned InitialVideoMode : 2;
    unsigned FloppyDrives : 2;
    bool     NoDma : 1;
    unsigned SerialPorts : 3;
    bool     GamePort : 1;
    bool     SerialPrinter : 1;
    unsigned ParallelPorts : 2;
} BIOS_EQUIPMENT;

static_assert(sizeof(uint16_t) == sizeof(BIOS_EQUIPMENT),
              "BIOS equipment list size doesn't match specification");

#pragma pack(pop)

static inline uint16_t
BiosKeyboardGetKeystroke(void)
{
    uint16_t ax;
    asm volatile("int $0x16" : "=a"(ax) : "Rah"((uint8_t)0x00));
    return ax;
}

static inline void
BiosVideoSetMode(uint8_t mode)
{
    uint16_t ax;
    asm volatile("int $0x10" : "=a"(ax) : "0"(mode) : "cc", "bx", "cx", "dx");
}

static inline void
BiosVideoSetCursorPosition(uint8_t page, uint16_t position)
{
    asm volatile("int $0x10"
                 :
                 : "a"((uint16_t)(0x02 << 8)), "b"((uint16_t)page << 8),
                   "d"(position));
}

static inline void
BiosVideoWriteCharacter(uint8_t  page,
                        uint8_t  character,
                        uint8_t  attribute,
                        uint16_t count)
{
    asm volatile("int $0x10"
                 :
                 : "a"((uint16_t)(0x09 << 8) | character),
                   "b"((uint16_t)(page << 8) | attribute), "c"(count));
}

static inline uint16_t
BiosVideoGetMode()
{
    uint16_t ax;
    asm volatile("int $0x10"
                 : "=a"(ax)
                 : "Rah"((uint8_t)0x0F)
                 : "cc", "bx", "cx", "dx");
    return ax;
}

static inline short
BiosVideoVbeDcCapabilities()
{
    unsigned short ax;
    asm volatile("int $0x10" : "=a"(ax) : "a"(0x4F15), "b"(0));
    return ax;
}

static inline short
BiosGetEquipmentList(void)
{
    unsigned short ax;
    asm volatile("int $0x11" : "=a"(ax));
    return ax;
}

static inline short
BiosVideoVbeDcReadEdid(far EDID *edid)
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

#endif // _API_BIOS_H
