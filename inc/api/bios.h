#ifndef _API_BIOS_H
#define _API_BIOS_H

#include <i86.h>

#include <fmt/edid.h>
#include <ker.h>

inline uint16_t
BiosKeyboardGetKeystroke(void)
{
    uint16_t ax;
    asm volatile("int $0x16" : "=a"(ax) : "Rah"((uint8_t)0x00));
    return ax;
}

inline void
BiosVideoSetMode(uint8_t mode)
{
    uint16_t ax;
    asm volatile("int $0x10" : "=a"(ax) : "0"(mode) : "cc", "bx", "cx", "dx");
}

inline void
BiosVideoSetCursorPosition(uint8_t page, uint16_t position)
{
    asm volatile("int $0x10"
                 :
                 : "Rah"((uint8_t)0x02), "b"((uint16_t)page << 8),
                   "d"(position));
}

inline void
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

inline uint16_t
BiosVideoGetMode()
{
    uint16_t ax;
    asm volatile("int $0x10"
                 : "=a"(ax)
                 : "Rah"((uint8_t)0x0F)
                 : "cc", "bx", "cx", "dx");
    return ax;
}

inline short
BiosVideoVbeDcCapabilities()
{
    unsigned short ax;
    asm volatile("int $0x10" : "=a"(ax) : "a"(0x4F15), "b"(0));
    return ax;
}

inline short
BiosVideoVbeDcReadEdid(far EDID *edid)
{
    unsigned short ax, es, di;
    es = FP_SEG(edid);
    di = FP_OFF(edid);
    asm volatile("mov %%bx, %%es; mov $1, %%bx; int $0x10"
                 : "=a"(ax)
                 : "a"(0x4F15), "b"(es), "D"(di)
                 : "c", "d", "memory");
    return ax;
}

#endif // _API_BIOS_H
