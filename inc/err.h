#ifndef _ERR_H_
#define _ERR_H_

#define ERR_FACILITY_KER 0
#define ERR_FACILITY_VID 1
#define ERR_FACILITY_GFX 2
#define ERR_FACILITY_SLD 3

#define ERR_OK 0

#define ERR_CODE(facility, code) (((facility) << 5) | ((code)&0b11111))

#ifndef __ASSEMBLER__

#define ERR(e)                                                                 \
    {                                                                          \
        return -(ERR_##e);                                                     \
    }

void
KerTerminate(int error);

#endif // __ASSEMBLER__

#endif // _ERR_H_
