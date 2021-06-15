%define KER_API_SUBSET_INT
%include "ker.inc"


                cpu     8086


[bits 16]
section .text


                global  KerInstallIsr
KerInstallIsr:
                shl     di, 1
                shl     di, 1               ; 4-byte alignment
                push    ax
                push    es

                cli
                xor     ax, ax
                mov     es, ax              ; ES = 0000h
                push    word [es:di]        ; Get current ISR offset
                push    word [es:di + 2]    ; Get current ISR segment
                pop     word [cs:bx + 2]
                pop     word [cs:bx]
                mov     word [es:di], si
                mov     word [es:di + 2], cs
                sti

                pop     es
                pop     ax
                ret


                global  KerUninstallIsr
KerUninstallIsr:
                shl     di, 1
                shl     di, 1               ; 4-byte alignment
                push    ax
                push    es

                cli
                xor     ax, ax
                mov     es, ax              ; ES = 0000h
                push    word [cs:si + 2]
                push    word [cs:si]
                pop     word [es:di]        ; Get current ISR offset
                pop     word [es:di + 2]    ; Get current ISR segment
                sti

                pop     es
                pop     ax
                ret
