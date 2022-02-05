#include <crg.h>


.section .rodata

                                .global StrCrgEnterPassword
StrCrgEnterPassword:            .asciz  "Enter Password"

                                .global StrCrgEncryptedLine1
StrCrgEncryptedLine1:           .asciz  "This content is encrypted."

                                .global StrCrgEncryptedLine2
StrCrgEncryptedLine2:           .asciz  "You need to provide the access key."


.section .errf


.section .errm

