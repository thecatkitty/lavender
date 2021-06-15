%ifndef UNI_API

%define UNI_API extern

; Get code point from UTF-8 sequence
;   CURRENTLY WORKS ONLY FOR CODE POINTS UNDER 0800h
; Input:
;   DS:SI - UTF-8 sequence
; Output:
;   AX    - code point under 0800h
;   DS:SI - first byte after the end of sequence
;   CF    - error
UNI_API KerGetCharacterFromUtf8

; Conduct a case-insensitive comparison of two UTF-8 strings
; CURRENTLY WORKS ONLY FOR LATIN SCRIPT UNDER 0800h
; Input:
;   DS:SI - left string
;   DS:BX - right string
;   CX    - length in bytes
; Output:
;   ZF    - difference
;   CF    - error
; Invalidates: 
UNI_API KerCompareUtf8IgnoreCase

; Get case folding for Unicode code point
;   CURRENTLY WORKS ONLY FOR CODE POINTS UNDER 0800h
; Input:
;   AX    - code point
; Output:
;   AX    - case folding first code point
;   BX    - case folding second code point or NUL
UNI_API KerFoldCase

%endif
