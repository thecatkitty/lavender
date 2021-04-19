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

%endif
