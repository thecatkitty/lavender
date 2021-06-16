%define KER_API_SUBSET_ERROR
%include "err.inc"
%include "ker.inc"
%include "api/dos.inc"


                cpu     8086

                extern  ErrFacilities
                extern  ErrMessages

[bits 16]
section .text

                global  KerTerminate
KerTerminate:
                mov     ah, DOS_PUTS
                mov     dx, sErrHeader
                int     DOS_INT

                mov     al, byte [KerLastError]
                mov     cl, 5
                shr     al, cl
                mov     si, ErrFacilities
                call    ErrFindMessage
                mov     dx, si
                int     DOS_INT

                mov     ah, DOS_PUTS
                mov     dx, sErrSeparator
                int     DOS_INT

                mov     al, byte [KerLastError]
                mov     si, ErrMessages
                call    ErrFindMessage
                mov     dx, si
                int     DOS_INT

                mov     al, byte [KerLastError]
                mov     ah, DOS_EXIT
                int     DOS_INT


; Find a message using its key byte
;   WILL CRASH IF MESSAGE NOT FOUND!
; Input:
;   AL    - key
;   DS:SI - begin of message list
; Output:
;   DS:SI - found message
ErrFindMessage:
                cmp     byte [si], al
                je      .End
.NextCharacter:
                inc     si
                cmp     byte [si], '$'
                jne     .NextCharacter
                inc     si
                jmp     ErrFindMessage
.End:
                inc     si
                ret


section .data


sErrHeader                      db      "ERROR: $"
sErrSeparator                   db      " - $"


section .bss


                                global  KerLastError
KerLastError                    resb    1
