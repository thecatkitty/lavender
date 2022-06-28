#ifndef _API_DOS_H
#define _API_DOS_H

#include <dos.h>

#include <base.h>

static inline void
dos_putc(int c)
{
    bdos(0x02, c, 0);
}

static inline void
dos_puts(const char *str)
{
    bdos(0x09, (unsigned)str, 0);
}

static inline unsigned
dos_get_version(void)
{
    return bdos(0x30, 0, 0);
}

static inline void
dos_exit(int code)
{
    bdos(0x4C, 0, code);
    while (1)
        ;
}

static inline unsigned
dos_read_disk(uint8_t drive, uint16_t sectors, uint16_t start, char *buffer)
{
    unsigned short ax;
    asm volatile("int $0x25; popf"
                 : "=a"(ax)
                 : "Ral"(drive), "c"(sectors), "d"(start), "b"(buffer)
                 : "memory");
    return ax;
}

#endif // _API_DOS_H
