#include <err.inc>


.section .ctors.errf


.byte   ERR_FACILITY_GFX
.ascii  "Graphics$"


.section .ctors.errm


.byte   ERR_GFX_FORMAT
.ascii  "Unsupported format$"

.byte   ERR_GFX_MALFORMED_FILE
.ascii  "Malformed file$"
