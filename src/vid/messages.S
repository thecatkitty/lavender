#include <err.inc>


.section .ctors.errf


.byte   ERR_FACILITY_VID
.ascii  "Video$"


.section .ctors.errm


.byte   ERR_VID_UNSUPPORTED
.ascii  "Unsupported feature requested$"

.byte   ERR_VID_FAILED
.ascii  "Operation failed$"

.byte   ERR_VID_FORMAT
.ascii  "Improper graphics format$"
