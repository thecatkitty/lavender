#ifndef _KER_H_
#define _KER_H_

#ifndef __ASSEMBLER__

#include <base.h>

#define interrupt __attribute__((interrupt))

typedef void interrupt far (*ISR)(void);

#define EXIT_ERRNO 512

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_KER_UNSUPPORTED       ERR_CODE(ERR_FACILITY_KER, 1)
#define ERR_KER_NOT_FOUND         ERR_CODE(ERR_FACILITY_KER, 2)
#define ERR_KER_ARCHIVE_NOT_FOUND ERR_CODE(ERR_FACILITY_KER, 3)
#define ERR_KER_INVALID_SEQUENCE  ERR_CODE(ERR_FACILITY_KER, 6)

#endif // _KER_H_
