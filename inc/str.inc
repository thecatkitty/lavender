%ifndef STR_API

%define STR_API extern

; Parse a space-delimited unsigned 16-bit integer
; Input:
;   DS:SI - string
; Output:
;   AX    - parsed integer
;   CX    - field length
;   CF    - parsing error
;   DS:SI - first cell after the field
STR_API StrParseU16

; Compare two memory ranges
; Input:
;   DS:SI - first range
;   DS:BX - second range
;   CX    - range length
; Output:
;   CF, ZF - difference
STR_API StrCompareMemory

%endif
