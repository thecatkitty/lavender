%ifndef KER_API

%define KER_API extern

; Check if running inside DOSBox
; Output:
;   ZF - 0 if DOSBox
; Invalidates: CX
KER_API KerIsDosBox

; Halt execution for a given period
; Input:
;   CX - number of ticks (1 tick ~ 54,9 ms)
; Invalidates: CX
KER_API KerSleep

%endif

KER_PIC1_IO                     equ     20h
KER_PICx_IO_COMMAND             equ     0
KER_PICx_IO_DATA                equ     1
KER_PIC1_IO_COMMAND             equ     KER_PIC1_IO + KER_PICx_IO_COMMAND
KER_PIC1_IO_DATA                equ     KER_PIC1_IO + KER_PICx_IO_DATA

KER_PIC_COMMAND_EOI             equ     20h

KER_INT_PIT                     equ     08h
KER_PIT_IO                      equ     40h
KER_PIT_IO_COMMAND              equ     KER_PIT_IO + 3

KER_PIT_CHANNEL                 equ     6
KER_PIT_READ_BACK               equ     3
KER_PIT_CHANNEL_MASK            equ     11b

KER_PIT_BYTE_HI                 equ     5
KER_PIT_BYTE_LO                 equ     4

KER_PIT_MODE                    equ     1
KER_PIT_MODE_INT_TERMINAL_COUNT equ     0
KER_PIT_MODE_HW_ONE_SHOT        equ     1
KER_PIT_MODE_RATE_GEN           equ     2
KER_PIT_MODE_SQUARE_WAVE_GEN    equ     3
KER_PIT_MODE_SW_STROBE          equ     4
KER_PIT_MODE_HW_STROBE          equ     5
KER_PIT_MODE_MASK               equ     111b

KER_PIT_BCD                     equ     0

KER_PIT_INPUT_FREQ              equ     1193181_6667
KER_PIT_FREQ_DIVISOR            equ     2048
KER_DELAY_MS_MULTIPLIER         equ     100
KER_DELAY_MS_DIVISOR            equ     (1000_0000 * KER_DELAY_MS_MULTIPLIER) \
                                        * KER_PIT_FREQ_DIVISOR / KER_PIT_INPUT_FREQ
