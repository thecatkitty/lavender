#include <sld.h>


.section .errf


.byte   ERR_FACILITY_SLD
.ascii  "Slides$"


.section .errm


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

.byte   ERR_SLD_LABEL_NOT_FOUND
.ascii  "Label not found$"
