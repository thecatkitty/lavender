#ifndef _API_WINOLDAP_H_
#define _API_WINOLDAP_H_

#include <i86.h>

#include <base.h>

static inline unsigned
winoldap_get_version(void)
{
    unsigned ax;
    asm volatile("int $0x2F" : "=a"(ax) : "a"(0x1700) : "memory", "cc");
    return ax;
}

static inline unsigned
winoldap_set_title(far const char *title)
{
    unsigned ax;
    asm volatile("push %%es; mov %%si, %%es; int $0x2F; pop %%es"
                 : "=a"(ax)
                 : "a"(0x168E), "d"(0), "D"(FP_OFF(title)), "S"(FP_SEG(title))
                 : "memory", "cc");
    return ax;
}

static inline short
winoldap_set_closable(unsigned state)
{
    unsigned ax;
    asm volatile("int $0x2F"
                 : "=a"(ax)
                 : "a"(0x168F), "d"(0x0000 | (state & 0xFF))
                 : "memory", "cc");
    return ax;
}

static inline short
winoldap_query_close(void)
{
    unsigned ax;
    asm volatile("int $0x2F"
                 : "=a"(ax)
                 : "a"(0x168F), "d"(0x0100)
                 : "memory", "cc");
    return ax;
}

static inline short
winoldap_acknowledge_close(void)
{
    unsigned ax;
    asm volatile("int $0x2F"
                 : "=a"(ax)
                 : "a"(0x168F), "d"(0x0200)
                 : "memory", "cc");
    return ax;
}

#endif // _API_WINOLDAP_H_
