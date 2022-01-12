#ifndef _VID_H_
#define _VID_H_

#include <stdint.h>

#include <err.h>
#include <fmt/edid.h>

#define ERR_VID_UNSUPPORTED             ERR_CODE(ERR_FACILITY_VID, 0)
#define ERR_VID_FAILED                  ERR_CODE(ERR_FACILITY_VID, 1)

// Get pixel aspect ratio
// Returns pixel aspect ratio (PAR = 64 / value), default value on error
extern uint16_t VidGetPixelAspectRatio(void);

#endif // _VID_H_
