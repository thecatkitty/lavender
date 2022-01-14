#include <err.inc>


.section .ctors.errf


.byte   ERR_FACILITY_SLD
.ascii  "Slides$"


.section .ctors.errm


.byte   ERR_SLD_INVALID_DELAY
.ascii  "Invalid delay$"

.byte   ERR_SLD_UNKNOWN_TYPE
.ascii  "Unknown type$"

.byte   ERR_SLD_INVALID_VERTICAL
.ascii  "Invalid vertical position$"

.byte   ERR_SLD_INVALID_HORIZONTAL
.ascii  "Invalid horizontal position$"

.byte   ERR_SLD_CONTENT_TOO_LONG
.ascii  "Content too long$"
