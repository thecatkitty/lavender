#include <sld.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_SLD
IF_LANG(ENU, .ascii  "Slides$")
IF_LANG(PLP, .ascii  "Slajdy$")


.section .errm


.byte   ERR_SLD_INVALID_DELAY
IF_LANG(ENU, .ascii  "Invalid delay$")
IF_LANG(PLP, .ascii  "Niepoprawne opoznienie$")

.byte   ERR_SLD_UNKNOWN_TYPE
IF_LANG(ENU, .ascii  "Unknown type$")
IF_LANG(PLP, .ascii  "Nieznany typ$")

.byte   ERR_SLD_INVALID_VERTICAL
IF_LANG(ENU, .ascii  "Invalid vertical position$")
IF_LANG(PLP, .ascii  "Niepoprawne polozenie pionowe$")

.byte   ERR_SLD_INVALID_HORIZONTAL
IF_LANG(ENU, .ascii  "Invalid horizontal position$")
IF_LANG(PLP, .ascii  "Niepoprawne polozenie poziome$")

.byte   ERR_SLD_CONTENT_TOO_LONG
IF_LANG(ENU, .ascii  "Content too long$")
IF_LANG(PLP, .ascii  "Zawartosc za dluga$")

.byte   ERR_SLD_LABEL_NOT_FOUND
IF_LANG(ENU, .ascii  "Label not found$")
IF_LANG(PLP, .ascii  "Nie znaleziono etykiety$")

.byte   ERR_SLD_INVALID_COMPARISON
IF_LANG(ENU, .ascii  "Invalid value in comparison$")
IF_LANG(PLP, .ascii  "Niepoprawna wartosc w porownaniu$")
