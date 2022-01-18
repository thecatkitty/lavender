#include <gfx.h>


.section .errf


.byte   ERR_FACILITY_GFX
.ascii  "Graphics$"


.section .errm


.byte   ERR_GFX_FORMAT
.ascii  "Unsupported format$"

.byte   ERR_GFX_MALFORMED_FILE
.ascii  "Malformed file$"
