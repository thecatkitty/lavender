%ifndef KER_API

%define KER_API extern

; Halt execution for a given period
; Input:
;   CX - number of ticks (1 tick ~ 54,9 ms)
; Invalidates: CX
KER_API KerSleep

%endif
