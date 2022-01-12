#ifndef _API_BIOS_H
#define _API_BIOS_H

#include <i86.h>

#include <fmt/edid.h>
#include <ker.h>

inline void
BiosVideoSetMode(uint8_t mode)
{
    uint16_t ax;
    asm volatile("int $0x10" : "=a"(ax) : "0"(mode) : "cc", "bx", "cx", "dx");
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
