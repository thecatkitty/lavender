#include <vid.h>


.section .errf


.byte   ERR_FACILITY_VID
.ascii  "Video$"


.section .errm


.byte   ERR_VID_UNSUPPORTED
.ascii  "Unsupported feature requested$"

.byte   ERR_VID_FAILED
.ascii  "Operation failed$"

.byte   ERR_VID_FORMAT
.ascii  "Improper graphics format$"
