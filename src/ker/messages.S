#include <ker.h>
#include <nls.h>


.section .rodata


                                .global StrKerError
StrKerError:
IF_LANG(ENU,                    .ascii  "ERROR: $")
IF_LANG(PLP,                    .ascii  "BLAD: $")


.section .errf


.byte   ERR_FACILITY_KER
IF_LANG(ENU, .ascii  "Kernel$")
IF_LANG(PLP, .ascii  "Rdzen$")


.section .errm


.byte   ERR_OK
.ascii  "OK$"

.byte   ERR_KER_UNSUPPORTED
IF_LANG(ENU, .ascii  "Unsupported feature requested$")
IF_LANG(PLP, .ascii  "Zazadano niewspieranej funkcjonalnosci$")

.byte   ERR_KER_NOT_FOUND
IF_LANG(ENU, .ascii  "Item not found$")
IF_LANG(PLP, .ascii  "Nie znaleziono elementu$")

.byte   ERR_KER_ARCHIVE_NOT_FOUND
IF_LANG(ENU, .ascii  "Archive not found$")
IF_LANG(PLP, .ascii  "Nie znaleziono archiwum$")

.byte   ERR_KER_ARCHIVE_TOO_LARGE
IF_LANG(ENU, .ascii  "Archive is too large$")
IF_LANG(PLP, .ascii  "Archiwum jest za duze$")

.byte   ERR_KER_ARCHIVE_INVALID
IF_LANG(ENU, .ascii  "Archive is invalid$")
IF_LANG(PLP, .ascii  "Archiwum jest niepoprawne$")

.byte   ERR_KER_INVALID_SEQUENCE
IF_LANG(ENU, .ascii  "Invalid or unsupported UTF-8 sequence$")
IF_LANG(PLP, .ascii  "Niepoprawna lub niewspierana sekwencja UTF-8$")

.byte   ERR_KER_INTEGRITY
IF_LANG(ENU, .ascii  "Data corrupted$")
IF_LANG(PLP, .ascii  "Dane uszkodzone$")
