%define KER_API_SUBSET_STRING
%include "ker.inc"


                cpu     8086

[bits 16]
section .text


                global  KerByteToHex
KerByteToHex:
                push    bx
                xor     bx, bx
                mov     bl, al
                and     bl, 0Fh

                and     ax, 00F0h
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                or      ax, bx
                add     ax, ('0' << 8) | '0'

                cmp     al, '9'
                jbe     .CheckHigh
                add     al, 'A' - '9' - 1
.CheckHigh:
                cmp     ah, '9'
                jbe     .End
                add     ah, 'A' - '9' - 1
.End:
                pop     bx
                ret
