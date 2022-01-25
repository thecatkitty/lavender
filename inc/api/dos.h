#ifndef _API_DOS_H
#define _API_DOS_H

#include <dos.h>

#include <base.h>

#pragma pack(push, 1)
typedef struct _DOS_PSP
{
    uint8_t  Int20H[2];
    uint16_t MemoryEndSegment;
    uint8_t  _Reserved1;
    union {
        uint8_t b[5];
        struct
        {
            uint8_t  _Reserved1;
            uint16_t ComSegmentSize;
            uint8_t  _Reserved2[2];
        } s;
    } DispatcherCall;
    far void *PreviousTerminate;
    far void *PreviousBreak;
    far void *PreviousCritical;
    uint16_t  ParentPspSegment;
    uint8_t   JobFileTable[20];
    uint16_t  EnvironmentSegment;
    far void *Int21HSsSp;
    uint16_t  JobFileTableSize;
    far uint8_t *JobFileTablePointer;
    far uint8_t *PreviousPsp;
    uint8_t      _Reserved2[4];
    uint16_t     DosVersion;
    uint8_t      _Reserved3[14];
    uint8_t      FarCallEntry[3];
    uint8_t      _Reserved4[2];
    uint8_t      _Reserved5[7];
    uint8_t      File1[16];
    uint8_t      File2[20];
    uint8_t      CommandLineLength;
    uint8_t      CommandLine[127];
} DOS_PSP;
#pragma pack(pop)

inline void
DosPutC(int c)
{
    bdos(0x02, c, 0);
}

inline void
DosPutS(const char *str)
{
    bdos(0x09, (unsigned)str, 0);
}

inline unsigned
DosGetVersion(void)
{
    return bdos(0x30, 0, 0);
}

inline void
DosExit(int code)
{
    bdos(0x4C, 0, code);
    while (1)
        ;
}

#endif // _API_DOS_H
