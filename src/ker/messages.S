#include <ker.h>


.section .errf


.byte   ERR_FACILITY_KER
.ascii  "Kernel$"


.section .errm


.byte   ERR_OK
.ascii  "OK$"

.byte   ERR_KER_UNSUPPORTED
.ascii  "Unsupported feature requested$"

.byte   ERR_KER_NOT_FOUND
.ascii  "Item not found$"

.byte   ERR_KER_ARCHIVE_NOT_FOUND
.ascii  "Archive not found$"

.byte   ERR_KER_ARCHIVE_TOO_LARGE
.ascii  "Archive is too large$"

.byte   ERR_KER_ARCHIVE_INVALID
.ascii  "Archive is invalid$"

.byte   ERR_KER_INVALID_SEQUENCE
.ascii  "Invalid or unsupported UTF-8 sequence$"
