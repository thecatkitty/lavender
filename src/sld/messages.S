#include <sld.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_SLD
IF_LANG(ENU, .ascii  "Slides$")


.section .errm


.byte   ERR_SLD_INVALID_DELAY
IF_LANG(ENU, .ascii  "Invalid delay$")

.byte   ERR_SLD_UNKNOWN_TYPE
IF_LANG(ENU, .ascii  "Unknown type$")

.byte   ERR_SLD_INVALID_VERTICAL
IF_LANG(ENU, .ascii  "Invalid vertical position$")

.byte   ERR_SLD_INVALID_HORIZONTAL
IF_LANG(ENU, .ascii  "Invalid horizontal position$")

.byte   ERR_SLD_CONTENT_TOO_LONG
IF_LANG(ENU, .ascii  "Content too long$")

.byte   ERR_SLD_LABEL_NOT_FOUND
IF_LANG(ENU, .ascii  "Label not found$")

.byte   ERR_SLD_INVALID_COMPARISON
IF_LANG(ENU, .ascii  "Invalid value in comparison$")
