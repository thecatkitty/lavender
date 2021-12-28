%define KER_API_SUBSET_TIME
%include "ker.inc"
%include "dev/pic.inc"
%include "dev/pit.inc"


                cpu     8086


[bits 16]
section .text


                global  KerSleep
KerSleep:
                jcxz    .End
                push    ax
                mov     ax, [cs:PitIsr.wCounter]
.Next:
                hlt
                cmp     ax, [cs:PitIsr.wCounter]
                je      .Next
                loop    .Next
                pop     ax

.End:           ret


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

                mov     dx, PIT_IO
                add     dl, al
                mov     cl, PIT_CHANNEL
                shl     al, cl
                or      al, (1 << PIT_BYTE_HI) | (1 << PIT_BYTE_LO)
                and     ah, PIT_MODE_MASK
                shl     ah, PIT_MODE
                or      al, ah

                cli
                out     PIT_IO_COMMAND, al
                mov     ax, bx              ; Set frequency divisor
                out     dx, al              ;   low byte
                mov     al, ah
                out     dx, al              ;   high byte
                sti

                pop     dx
                pop     cx
.End:           ret


; Initialize Programmable Interval Timer
                global  PitInitialize
PitInitialize:
                mov     ax, INT_PIT
                push    ax
                push    cs
                mov     ax, PitIsr
                push    ax
                call    _KerInstallIsr
                add     sp, 6
                mov     word [PitIsr.lpfnBiosIsr], ax
                mov     word [PitIsr.lpfnBiosIsr + 2], dx

                xor     al, al              ; Channel 0
                mov     ah, PIT_MODE_RATE_GEN
                mov     bx, KER_PIT_FREQ_DIVISOR
                call    PitInitChannel
                ret


; Deinitialize Programmable Interval Timer
                global  PitDeinitialize
PitDeinitialize:
                mov     ax, INT_PIT
                push    ax
                mov     ax, word [PitIsr.lpfnBiosIsr + 2]
                push    ax
                mov     ax, word [PitIsr.lpfnBiosIsr]
                push    ax
                call    _KerUninstallIsr
                add     sp, 6

                xor     al, al              ; Channel 0
                mov     ah, PIT_MODE_RATE_GEN
                xor     bx, bx              ; Fin / Fout = 65536
                call    PitInitChannel      ; Fout ~ 18,2065 Hz
                ret


; Programmable Interval Timer interrupt service routine
PitIsr:
                cli
                inc     word [cs:.wCounter]
                test    word [cs:.wCounter], 11111b
                jnz     .End
                jmp     far [cs:.lpfnBiosIsr]      ; Call BIOS timer ISR every 32 ticks (~18,2065 Hz)
.End:
                push    ax
                mov     al, PIC1_IO_COMMAND
                out     PIC_COMMAND_EOI, al
                pop     ax
                sti
                iret
.lpfnBiosIsr    dd      0
.wCounter       dw      0FFFFh
