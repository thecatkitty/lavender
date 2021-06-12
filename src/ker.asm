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
                ; Initialize stack
                mov     sp, StackBottom

                ; Initialize Programmable Interval Timer
                mov     si, PitIsr
                mov     di, KER_INT_PIT
                mov     bx, PitIsr.BiosIsr
                call    IsrInstall

                xor     al, al              ; Channel 0
                mov     ah, KER_PIT_MODE_RATE_GEN
                mov     bx, KER_PIT_FREQ_DIVISOR
                call    PitInitChannel

                call    Main
                ; Pass to KerExit


                global  KerExit
KerExit:
                mov     di, KER_INT_PIT
                mov     si, PitIsr.BiosIsr
                call    IsrUninstall

                xor     al, al              ; Channel 0
                mov     ah, KER_PIT_MODE_RATE_GEN
                xor     bx, bx              ; Fin / Fout = 65536
                call    PitInitChannel      ; Fout ~ 18,2065 Hz
                mov     ah, DOS_EXIT
                int     DOS_INT


                global  KerSleep
KerSleep:
                jcxz    .End
                push    ax
                mov     ax, [cs:PitIsr.Counter]
.Next:
                hlt
                cmp     ax, [cs:PitIsr.Counter]
                je      .Next
                loop    .Next
                pop     ax

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


; Install interrupt service routine
; Input:
;   CS:SI - interrupt service routine
;   DI    - interrupt number
;   CS:BX - output pointer
; Output:
;   CS:BX - long pointer to previous interrupt service routine
; Invalidates: DI
IsrInstall:
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


; Uninstall interrupt service routine
; Input:
;   DI    - interrupt number
;   CS:SI - pointer to pointer to previous interrupt service routine
; Invalidates: DI
IsrUninstall:
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


; Initialize one channel of the Programmable Interval Timer
; Input:
;   AL - channel number
;   AH - mode
;   BX - frequency divisor
; Invalidates: AX
PitInitChannel:
                cmp     al, 2
                ja      .End

                push    cx
                push    dx

                mov     dx, KER_PIT_IO
                add     dl, al
                mov     cl, KER_PIT_CHANNEL
                shl     al, cl
                or      al, (1 << KER_PIT_BYTE_HI) | (1 << KER_PIT_BYTE_LO)
                and     ah, KER_PIT_MODE_MASK
                shl     ah, KER_PIT_MODE
                or      al, ah

                cli
                out     KER_PIT_IO_COMMAND, al
                mov     ax, bx              ; Set frequency divisor
                out     dx, al              ;   low byte
                mov     al, ah
                out     dx, al              ;   high byte
                sti

                pop     dx
                pop     cx
.End:           ret


; Programmable Interval Timer interrupt service routine
PitIsr:
                cli
                inc     word [cs:.Counter]
                test    word [cs:.Counter], 11111b
                jnz     .End
                jmp     far [cs:.BiosIsr]      ; Call BIOS timer ISR every 32 ticks (~18,2065 Hz)
.End:
                push    ax
                mov     al, KER_PIC1_IO_COMMAND
                out     KER_PIC_COMMAND_EOI, al
                pop     ax
                sti
                iret
.BiosIsr        dd      0
.Counter        dw      0FFFFh
