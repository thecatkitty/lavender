#include <ker.h>
#include <nls.h>


.section .rodata


                                .global StrKerError
StrKerError:
IF_LANG(CSY,                    .ascii  "CHYBA: $")
IF_LANG(ENU,                    .ascii  "ERROR: $")
IF_LANG(PLK,                    .ascii  "BLAD: $")


.section .errf


.byte   ERR_FACILITY_KER
IF_LANG(CSY, .ascii  "Jadro$")
IF_LANG(ENU, .ascii  "Kernel$")
IF_LANG(PLK, .ascii  "Rdzen$")


.section .errm


.byte   ERR_OK
.ascii  "OK$"

.byte   ERR_KER_UNSUPPORTED
IF_LANG(CSY, .ascii  "Byla pozadovana nepodporovana funkce$")
IF_LANG(ENU, .ascii  "Unsupported feature requested$")
IF_LANG(PLK, .ascii  "Zazadano niewspieranej funkcjonalnosci$")

.byte   ERR_KER_NOT_FOUND
IF_LANG(CSY, .ascii  "Polozka nenalezena$")
IF_LANG(ENU, .ascii  "Item not found$")
IF_LANG(PLK, .ascii  "Nie znaleziono elementu$")

.byte   ERR_KER_ARCHIVE_NOT_FOUND
IF_LANG(CSY, .ascii  "Archiv nenalezen$")
IF_LANG(ENU, .ascii  "Archive not found$")
IF_LANG(PLK, .ascii  "Nie znaleziono archiwum$")

.byte   ERR_KER_INVALID_SEQUENCE
IF_LANG(CSY, .ascii  "Neplatna nebo nepodporovana sekvence UTF-8$")
IF_LANG(ENU, .ascii  "Invalid or unsupported UTF-8 sequence$")
IF_LANG(PLK, .ascii  "Niepoprawna lub niewspierana sekwencja UTF-8$")
