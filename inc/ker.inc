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
