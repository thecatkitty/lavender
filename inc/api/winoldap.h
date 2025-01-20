#ifndef _API_WINOLDAP_H_
#define _API_WINOLDAP_H_

static inline unsigned
winoldap_get_version(void)
{
    unsigned ax;
    asm volatile("int $0x2F" : "=a"(ax) : "a"(0x1700) : "memory", "cc");
    return ax;
}

#endif // _API_WINOLDAP_H_
