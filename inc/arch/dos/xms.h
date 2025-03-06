#ifndef _ARCH_DOS_XMS_H_
#define _ARCH_DOS_XMS_H_

#include <i86.h>

#include <base.h>

// Returned by 2Fh, AX=4300h
#define XMS_PRESENT 0x80

// Extended Memory Manager functions
#define XMSF_VERSION       0x00
#define XMSF_QUERYFREE     0x08
#define XMSF_ALLOC         0x09
#define XMSF_FREE          0x0A
#define XMSF_MOVE          0x0B
#define XMSF_LOCK          0x0C
#define XMSF_UNLOCK        0x0D
#define XMSF_GETHANDLEINFO 0x0E
#define XMSF_REALLOC       0x0F

// Extended Memory Manager errors returned in BL
#define XMSE_NOTIMPLEMENTED 0x80
#define XMSE_VDISKPRESENT   0x81
#define XMSE_BADA20         0x82
#define XMSE_OUTOFMEMORY    0xA0
#define XMSE_OUTOFHANDLES   0xA1
#define XMSE_BADHANDLE      0xA2
#define XMSE_BADSRC         0xA3
#define XMSE_BADSRCOFF      0xA4
#define XMSE_BADDST         0xA5
#define XMSE_BADDSTOFF      0xA6
#define XMSE_BADLENGTH      0xA7
#define XMSE_BADOVERLAP     0xA8
#define XMSE_PARITY         0xA9
#define XMSE_NOTLOCKED      0xAA
#define XMSE_LOCKED         0xAB
#define XMSE_TOOMANYLOCKS   0xAC
#define XMSE_BADLOCK        0xAD

#pragma pack(push, 2)
typedef struct
{
    uint32_t length;
    uint16_t src;
    uint32_t src_offset;
    uint16_t dst;
    uint32_t dst_offset;
} xms_move_args;
#pragma pack(pop)

static inline uint8_t
xms_detect(void)
{
    uint16_t ax;
    asm volatile("int $0x2F" : "=a"(ax) : "a"((uint16_t)0x4300));
    return ax & 0xFF;
}

static inline far void *
xms_get_entry_point(void)
{
    uint16_t bx, es;
    asm volatile("push %%es; int $0x2F; mov %%es, %%ax; pop %%es"
                 : "=a"(es), "=b"(bx)
                 : "a"((uint16_t)0x4310)
                 : "memory");
    return MK_FP(es, bx);
}

#endif // _ARCH_DOS_XMS_H_
