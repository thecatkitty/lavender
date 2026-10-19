#ifndef _ARCH_DOS_MSDOS_H_
#define _ARCH_DOS_MSDOS_H_

#include <dos.h>

#include <base.h>

static inline void
msdos_putc(int c)
{
    bdos(0x02, c, 0);
}

static inline void
msdos_puts(const char *str)
{
    bdos(0x09, (unsigned)str, 0);
}

static inline unsigned
msdos_get_version(void)
{
    return bdos(0x30, 0, 0);
}

static inline void
msdos_exit(int code)
{
    bdos(0x4C, 0, code);
    while (1)
        ;
}

#endif // _ARCH_DOS_MSDOS_H_
