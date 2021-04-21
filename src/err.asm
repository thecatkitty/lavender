%define ERR_API
%include "dos.inc"
%include "err.inc"


                cpu     8086

[bits 16]
section .text

                global  ErrTerminate
ErrTerminate:
                mov     ah, DOS_PUTS
                mov     dx, sErrHeader
                int     DOS_INT

                xor     cx, cx
                mov     cl, [ErrLastCode]
                mov     si, asErrMessages
                jcxz    .PrintMessage
.NextCharacter:
                cmp     byte [si], '$'
                je      .NextMessage
                inc     si
                jmp     .NextCharacter
.NextMessage:
                inc     si
                dec     cx
                jz      .PrintMessage
                jmp     .NextCharacter
.PrintMessage:
                mov     dx, si
                int     DOS_INT

                mov     al, [ErrLastCode]
                mov     ah, DOS_EXIT
                int     DOS_INT


section .data


sErrHeader                      db      "ERROR: $"
asErrMessages                   db      "OK$"
                                db      "Invalid line delay$"
                                db      "Unknown line type$"
                                db      "Invalid line vertical position$"
                                db      "Invalid line horizontal position$"
                                db      "ZIP central directory end not found$"
                                db      "ZIP central directory is too large$"
                                db      "ZIP central directory is invalid$"
                                db      "File not found in ZIP$"
                                db      "ZIP local file header is invalid$"


section .bss

                                global  ErrLastCode
ErrLastCode                     resb    1
