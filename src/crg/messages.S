#include <crg.h>
#include <nls.h>


.section .rodata

                                .global StrCrgEnterPassword
StrCrgEnterPassword:
IF_LANG(CSY,                    .asciz  "Vložte heslo")
IF_LANG(ENU,                    .asciz  "Enter Password")
IF_LANG(PLK,                    .asciz  "Wprowadź hasło")

                                .global StrCrgEncryptedLine1
StrCrgEncryptedLine1:
IF_LANG(CSY,                    .asciz  "Tento obsah je zašifrován.")
IF_LANG(ENU,                    .asciz  "This content is encrypted.")
IF_LANG(PLK,                    .asciz  "Ta zawartość jest zaszyfrowana.")

                                .global StrCrgEncryptedLine2
StrCrgEncryptedLine2:
IF_LANG(CSY,                    .asciz  "Zadejte přístupový klíč.")
IF_LANG(ENU,                    .asciz  "Please enter the access key.")
IF_LANG(PLK,                    .asciz  "Proszę podać klucz dostępu.")

                                .global StrCrgKeyInvalid
StrCrgKeyInvalid:
IF_LANG(CSY,                    .asciz  "Zadaný klíč je neplatný!")
IF_LANG(ENU,                    .asciz  "Provided key is invalid!")
IF_LANG(PLK,                    .asciz  "Wprowadzony klucz jest niepoprawny!")


.section .errf


.section .errm

