#include <gfx.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_GFX
IF_LANG(ENU, .ascii  "Graphics$")


.section .errm


.byte   ERR_GFX_FORMAT
IF_LANG(ENU, .ascii  "Unsupported format$")

.byte   ERR_GFX_MALFORMED_FILE
IF_LANG(ENU, .ascii  "Malformed file$")
