#ifndef _ERR_H_
#define _ERR_H_

#define ERR_FACILITY_KER 0
#define ERR_FACILITY_VID 1
#define ERR_FACILITY_GFX 2
#define ERR_FACILITY_SLD 3

#define ERR_OK 0

#define ERR_CODE(facility, code) (((facility) << 5) | ((code)&0b11111))

// Kernel error codes, to be removed when migration to errno finishes
#define ERR_KER_UNSUPPORTED       ERR_CODE(ERR_FACILITY_KER, 1)
#define ERR_KER_NOT_FOUND         ERR_CODE(ERR_FACILITY_KER, 2)
#define ERR_KER_ARCHIVE_NOT_FOUND ERR_CODE(ERR_FACILITY_KER, 3)
#define ERR_KER_INVALID_SEQUENCE  ERR_CODE(ERR_FACILITY_KER, 6)

#ifndef __ASSEMBLER__

#define ERR(e)                                                                 \
    {                                                                          \
        return -(ERR_##e);                                                     \
    }

#endif // __ASSEMBLER__

#endif // _ERR_H_
