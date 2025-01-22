#ifndef _ARCH_DOS_MSMOUSE_H_
#define _ARCH_DOS_MSMOUSE_H_

#include <base.h>

#define MSMOUSE_AREA_WIDTH  640
#define MSMOUSE_AREA_HEIGHT 200

static inline bool
msmouse_init(void)
{
    uint16_t ax;
    asm volatile("int $0x33" : "=a"(ax) : "a"(0x0000) : "b");
    return 0xFFFF == ax;
}

static inline void
msmouse_show(void)
{
    asm volatile("int $0x33" : : "a"(0x0001));
}

static inline void
msmouse_hide(void)
{
    asm volatile("int $0x33" : : "a"(0x0002));
}

static inline uint16_t
msmouse_get_status(uint16_t *x, uint16_t *y)
{
    uint16_t bx;
    asm volatile("int $0x33" : "=b"(bx), "=c"(*x), "=d"(*y) : "a"(0x0003));
    return bx;
}

#endif // _ARCH_DOS_MSMOUSE_H_
