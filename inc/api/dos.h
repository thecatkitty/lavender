#ifndef _API_DOS_H
#define _API_DOS_H

#include <dos.h>

inline void DosPutC(
    int c)
{
    bdos(0x02, c, 0);
}

inline void DosPutS(
    const char *str)
{
    bdos(0x09, (unsigned)str, 0);
}

inline void DosExit(
    int code)
{
    bdos(0x4C, 0, code);
}

#endif // _API_DOS_H
