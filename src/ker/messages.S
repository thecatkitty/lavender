#include <ker.h>
#include <nls.h>


.section .errf


.byte   ERR_FACILITY_KER
IF_LANG(ENU, .ascii  "Kernel$")


.section .errm


.byte   ERR_OK
.ascii  "OK$"

.byte   ERR_KER_UNSUPPORTED
IF_LANG(ENU, .ascii  "Unsupported feature requested$")

.byte   ERR_KER_NOT_FOUND
IF_LANG(ENU, .ascii  "Item not found$")

.byte   ERR_KER_ARCHIVE_NOT_FOUND
IF_LANG(ENU, .ascii  "Archive not found$")

.byte   ERR_KER_ARCHIVE_TOO_LARGE
IF_LANG(ENU, .ascii  "Archive is too large$")

.byte   ERR_KER_ARCHIVE_INVALID
IF_LANG(ENU, .ascii  "Archive is invalid$")

.byte   ERR_KER_INVALID_SEQUENCE
IF_LANG(ENU, .ascii  "Invalid or unsupported UTF-8 sequence$")

.byte   ERR_KER_INTEGRITY
IF_LANG(ENU, .ascii  "Data corrupted$")
