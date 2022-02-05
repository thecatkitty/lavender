#include <gfx.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_GFX
IF_LANG(ENU, .ascii  "Graphics$")
IF_LANG(PLP, .ascii  "Grafika$")


.section .errm


.byte   ERR_GFX_FORMAT
IF_LANG(ENU, .ascii  "Unsupported format$")
IF_LANG(PLP, .ascii  "Niewspierany format$")

.byte   ERR_GFX_MALFORMED_FILE
IF_LANG(ENU, .ascii  "Malformed file$")
IF_LANG(PLP, .ascii  "Niepoprawny plik$")
