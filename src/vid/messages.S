#include <vid.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_VID
IF_LANG(CSY, .ascii  "Video$")
IF_LANG(ENU, .ascii  "Video$")
IF_LANG(PLK, .ascii  "Wideo$")


.section .errm


.byte   ERR_VID_FORMAT
IF_LANG(CSY, .ascii  "Nevhodny graficky format$")
IF_LANG(ENU, .ascii  "Improper graphics format$")
IF_LANG(PLK, .ascii  "Niewlasciwy format grafiki$")
