#include <gfx.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_GFX
IF_LANG(CSY, .ascii  "Grafika$")
IF_LANG(ENU, .ascii  "Graphics$")
IF_LANG(PLK, .ascii  "Grafika$")


.section .errm


.byte   ERR_GFX_MALFORMED_FILE
IF_LANG(CSY, .ascii  "Poskozeny soubor$")
IF_LANG(ENU, .ascii  "Malformed file$")
IF_LANG(PLK, .ascii  "Niepoprawny plik$")
