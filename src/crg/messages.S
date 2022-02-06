#include <crg.h>
#include <nls.h>


.section .rodata

                                .global StrCrgEnterPassword
StrCrgEnterPassword:
IF_LANG(ENU,                    .asciz  "Enter Password")
IF_LANG(PLK,                    .asciz  "Wprowadź hasło")

                                .global StrCrgEncryptedLine1
StrCrgEncryptedLine1:
IF_LANG(ENU,                    .asciz  "This content is encrypted.")
IF_LANG(PLK,                    .asciz  "Ta zawartość jest zaszyfrowana.")

                                .global StrCrgEncryptedLine2
StrCrgEncryptedLine2:
IF_LANG(ENU,                    .asciz  "You need to provide the access key.")
IF_LANG(PLK,                    .asciz  "Musisz podać klucz dostępu.")


.section .errf


.section .errm

