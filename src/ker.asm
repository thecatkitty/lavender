%define KER_API
%include "dos.inc"
%include "ker.inc"


                cpu     8086

                extern  Main
                extern  StackBottom
                extern  StackTop

[bits 16]
section .init


                jmp     KerEntry
%push About
%defstr GIT_COMMIT      %!GIT_COMMIT
%defstr GIT_TAG         %!GIT_TAG
                db      0Dh, "Lavender ", GIT_TAG, '-', GIT_COMMIT, 1Ah
%pop


section .text


KerEntry:
                mov     sp, StackBottom
                call    Main
                mov     ah, DOS_EXIT
                int     DOS_INT


                global  KerSleep
KerSleep:
                jcxz    .End
.Next:
                hlt
                loop    .Next

.End:           ret


                global  KerIsDosBox
KerIsDosBox:
                push    ax
                push    ds
                mov     ax, 0F000h
                mov     ds, ax
                cmp     word [0E061h], 4F44h    ; DO
                jne     .End
                cmp     word [0E063h], 4253h    ; SB
                jne     .End
                cmp     word [0E065h], 786Fh    ; ox
.End:           pop     ds
                pop     ax
                ret
