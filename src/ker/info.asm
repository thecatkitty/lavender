%define KER_API_SUBSET_INFO
%include "ker.inc"


                cpu     8086


[bits 16]
section .text


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
