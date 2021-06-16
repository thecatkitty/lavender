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
                push    ds

                cli
                xor     ax, ax
                mov     ds, ax              ; ES = 0000h
                push    word [ds:di]        ; Get current ISR offset
                push    word [ds:di + 2]    ; Get current ISR segment
                pop     word [es:bx + 2]
                pop     word [es:bx]
                mov     word [ds:di], si
                mov     word [ds:di + 2], es
                sti

                pop     ds
                pop     ax
                ret


                global  KerUninstallIsr
KerUninstallIsr:
                shl     di, 1
                shl     di, 1               ; 4-byte alignment
                push    ax
                push    ds

                cli
                xor     ax, ax
                mov     ds, ax              ; ES = 0000h
                push    word [es:si + 2]
                push    word [es:si]
                pop     word [ds:di]        ; Get current ISR offset
                pop     word [ds:di + 2]    ; Get current ISR segment
                sti

                pop     ds
                pop     ax
                ret
