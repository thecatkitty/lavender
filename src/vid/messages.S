#include <vid.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_VID
IF_LANG(ENU, .ascii  "Video$")


.section .errm


.byte   ERR_VID_UNSUPPORTED
IF_LANG(ENU, .ascii  "Unsupported feature requested$")

.byte   ERR_VID_FAILED
IF_LANG(ENU, .ascii  "Operation failed$")

.byte   ERR_VID_FORMAT
IF_LANG(ENU, .ascii  "Improper graphics format$")
