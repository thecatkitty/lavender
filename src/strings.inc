%ifndef STRINGS_API

%define STRINGS_API extern

; Parse a space-delimited unsigned 16-bit integer
; Input:
;   DS:SI - string
; Output:
;   AX    - parsed integer
;   CX    - field length
;   CF    - parsing error
;   DS:SI - first cell after the field
STRINGS_API parse_uint16

%endif
