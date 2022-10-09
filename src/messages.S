#include <nls.h>


.section .rodata

                                .global IDS_ERROR
IDS_ERROR:
IF_LANG(CSY,                    .asciz  "CHYBA: ")
IF_LANG(ENU,                    .asciz  "ERROR: ")
IF_LANG(PLK,                    .asciz  "BLAD: ")

                                .global IDS_NOARCHIVE
IDS_NOARCHIVE:
IF_LANG(CSY,                    .asciz  "Archiv nenalezen")
IF_LANG(ENU,                    .asciz  "Archive not found")
IF_LANG(PLK,                    .asciz  "Nie znaleziono archiwum")
