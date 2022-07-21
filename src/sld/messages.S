#include <sld.h>
#include <nls.h>


.section .rodata

                                .global IDS_ENTERDSN
IDS_ENTERDSN:
IF_LANG(CSY,                    .asciz  "Zadejte sériové číslo disku")
IF_LANG(ENU,                    .asciz  "Enter Disk Serial Number")
IF_LANG(PLK,                    .asciz  "Wprowadź numer seryjny dysku")

                                .global IDS_ENTERDSN_DESC
IDS_ENTERDSN_DESC:
IF_LANG(CSY,                    .asciz  "Zadejte prosím sériové číslo\npřipojené k disketě programu.")
IF_LANG(ENU,                    .asciz  "Please type the serial number\nattached to Your program diskette.")
IF_LANG(PLK,                    .asciz  "Proszę podać numer seryjny dołączony\ndo dyskietki z programem.")

                                .global IDS_ENTERPASS
IDS_ENTERPASS:
IF_LANG(CSY,                    .asciz  "Vložte heslo")
IF_LANG(ENU,                    .asciz  "Enter Password")
IF_LANG(PLK,                    .asciz  "Wprowadź hasło")

                                .global IDS_ENTERPASS_DESC
IDS_ENTERPASS_DESC:
IF_LANG(CSY,                    .asciz  "Tento obsah je zašifrován.\n\nZadejte přístupový klíč.")
IF_LANG(ENU,                    .asciz  "This content is encrypted.\n\nPlease enter the access key.")
IF_LANG(PLK,                    .asciz  "Ta zawartość jest zaszyfrowana.\n\nProszę podać klucz dostępu.")

                                .global IDS_INVALIDKEY
IDS_INVALIDKEY:
IF_LANG(CSY,                    .asciz  "Zadaný klíč je neplatný!")
IF_LANG(ENU,                    .asciz  "Provided key is invalid!")
IF_LANG(PLK,                    .asciz  "Wprowadzony klucz jest niepoprawny!")


.section .errf


.byte   ERR_FACILITY_SLD
IF_LANG(CSY, .ascii  "Snimky$")
IF_LANG(ENU, .ascii  "Slides$")
IF_LANG(PLK, .ascii  "Slajdy$")


.section .errm


.byte   ERR_SLD_INVALID_DELAY
IF_LANG(CSY, .ascii  "Neplatne zpozdeni$")
IF_LANG(ENU, .ascii  "Invalid delay$")
IF_LANG(PLK, .ascii  "Niepoprawne opoznienie$")

.byte   ERR_SLD_UNKNOWN_TYPE
IF_LANG(CSY, .ascii  "Neznamy typ$")
IF_LANG(ENU, .ascii  "Unknown type$")
IF_LANG(PLK, .ascii  "Nieznany typ$")

.byte   ERR_SLD_INVALID_VERTICAL
IF_LANG(CSY, .ascii  "Neplatna vertikalni poloha$")
IF_LANG(ENU, .ascii  "Invalid vertical position$")
IF_LANG(PLK, .ascii  "Niepoprawne polozenie pionowe$")

.byte   ERR_SLD_INVALID_HORIZONTAL
IF_LANG(CSY, .ascii  "Neplatna horizontalni poloha$")
IF_LANG(ENU, .ascii  "Invalid horizontal position$")
IF_LANG(PLK, .ascii  "Niepoprawne polozenie poziome$")

.byte   ERR_SLD_CONTENT_TOO_LONG
IF_LANG(CSY, .ascii  "Obsah prilis dlouhy$")
IF_LANG(ENU, .ascii  "Content too long$")
IF_LANG(PLK, .ascii  "Zawartosc za dluga$")

.byte   ERR_SLD_LABEL_NOT_FOUND
IF_LANG(CSY, .ascii  "Stitek nenalezen$")
IF_LANG(ENU, .ascii  "Label not found$")
IF_LANG(PLK, .ascii  "Nie znaleziono etykiety$")

.byte   ERR_SLD_INVALID_COMPARISON
IF_LANG(CSY, .ascii  "Neplatna hodnota v porovnani$")
IF_LANG(ENU, .ascii  "Invalid value in comparison$")
IF_LANG(PLK, .ascii  "Niepoprawna wartosc w porownaniu$")
