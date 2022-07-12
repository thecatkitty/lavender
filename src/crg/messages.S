#include <crg.h>
#include <nls.h>


.section .rodata

                                .global StrCrgEnterPassword
StrCrgEnterPassword:
IF_LANG(CSY,                    .asciz  "Vložte heslo")
IF_LANG(ENU,                    .asciz  "Enter Password")
IF_LANG(PLK,                    .asciz  "Wprowadź hasło")

                                .global StrCrgEncrypted
StrCrgEncrypted:
IF_LANG(CSY,                    .asciz  "Tento obsah je zašifrován.\n\nZadejte přístupový klíč.")
IF_LANG(ENU,                    .asciz  "This content is encrypted.\n\nPlease enter the access key.")
IF_LANG(PLK,                    .asciz  "Ta zawartość jest zaszyfrowana.\n\nProszę podać klucz dostępu.")

                                .global StrCrgKeyInvalid
StrCrgKeyInvalid:
IF_LANG(CSY,                    .asciz  "Zadaný klíč je neplatný!")
IF_LANG(ENU,                    .asciz  "Provided key is invalid!")
IF_LANG(PLK,                    .asciz  "Wprowadzony klucz jest niepoprawny!")


.section .errf


.section .errm

