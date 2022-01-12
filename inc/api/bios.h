#ifndef _API_BIOS_H
#define _API_BIOS_H

#include <i86.h>

#include <ker.h>
#include <fmt/edid.h>


inline short BiosVideoVbeDcCapabilities()
{
    unsigned short ax;
    asm volatile (
        "int $0x10"
        : "=a"(ax)
        : "a"(0x4F15), "b"(0));
    return ax;
}

inline short BiosVideoVbeDcReadEdid(
    far EDID *edid)
{
    unsigned short ax, es, di;
    es = FP_SEG(edid);
    di = FP_OFF(edid);
    asm volatile (
        "mov %%bx, %%es; mov $1, %%bx; int $0x10"
        : "=a"(ax)
        : "a"(0x4F15), "b"(es), "D"(di)
        : "c", "d", "memory");
    return ax;
}

#endif // _API_BIOS_H
