#include <vid.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_VID
IF_LANG(ENU, .ascii  "Video$")
IF_LANG(PLK, .ascii  "Wideo$")


.section .errm


.byte   ERR_VID_UNSUPPORTED
IF_LANG(ENU, .ascii  "Unsupported feature requested$")
IF_LANG(PLK, .ascii  "Zazadano niewspieranej funkcjonalnosci$")

.byte   ERR_VID_FAILED
IF_LANG(ENU, .ascii  "Operation failed$")
IF_LANG(PLK, .ascii  "Operacja nie powiodla sie$")

.byte   ERR_VID_FORMAT
IF_LANG(ENU, .ascii  "Improper graphics format$")
IF_LANG(PLK, .ascii  "Niewlasciwy format grafiki$")
