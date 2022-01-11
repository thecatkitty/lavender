%include "err.inc"

%macro  API_SUBSET 1
        %ifdef  KER_API_SUBSET_%1
                %define KER_API global
        %else
                %define KER_API extern
        %endif
%endmacro

; ----- System information functions -----
API_SUBSET INFO

; Check if running inside DOSBox
; Output:
;   AX - 1 if DOSBox
; Invalidates: AX, BX, CX, DX
KER_API KerIsDosBox


; ----- Time related functions -----
API_SUBSET TIME

; Halt execution for a given period
; Stack input:
;   number of ticks (1 tick ~ 54,9 ms)
; Invalidates: AX, BX, CX, DX
extern KerSleep

; ----- Interrupt related functions -----
; Install interrupt service routine
; Stack input:
;   interrupt service routine offset
;   interrupt service routine segment
;   interrupt number
; Output:
;   AX - previous interrupt service routine offset
;   DX - previous interrupt service routine segment
; Invalidates: AX, BX, CX, DX
extern KerInstallIsr

; Uninstall interrupt service routine
; Stack input:
;   previous interrupt service routine offset
;   previous interrupt service routine segment
;   interrupt number
; Invalidates: AX, BX, CX, DX
extern KerUninstallIsr


; ----- String and memory manipulation functions -----
API_SUBSET STRING

; Parse a space-delimited unsigned 16-bit integer
; Input:
;   DS:SI - string
; Output:
;   AX    - parsed integer
;   CX    - field length
;   CF    - parsing error
;   DS:SI - first cell after the field
KER_API KerParseU16

; Convert byte to its hexadecimal representation
; Input:
;   AL    - byte
; Output:
;   AH    - first hexadecimal digit
;   AL    - second hexadecimal digit
KER_API KerByteToHex

; Tells if given byte is a whitespace character
; Input:
;   AL - byte
; Output:
;   ZF - zero if a whitespace
KER_API KerIsWhitespace


; ----- Unicode support functions -----
API_SUBSET UNICODE

; Get code point from UTF-8 sequence
;   CURRENTLY WORKS ONLY FOR CODE POINTS UNDER 0800h
; Stack input:
;   near pointer to the UTF-8 sequence
;   near pointer to output WORD
; Output:
;   AX - length of the sequence (0 when NUL), negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerGetCharacterFromUtf8

; Conduct a case-insensitive comparison of two UTF-8 strings
;   CURRENTLY WORKS ONLY FOR CODE POINTS UNDER 0800h
; Stack input:
;   near pointer to the left string
;   near pointer to the right string
;   length in bytes
; Output:
;   AX - 0 when equal, 1 when different, negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerCompareUtf8IgnoreCase


; ----- ZIP archive functions -----
API_SUBSET ZIP

; Locate ZIP central directory end structure
; Stack input:
;   search range lower boundary (near)
;   search range upper boundary (near)
;   near pointer to output structure pointer
; Output:
;   AX - 0 when found, negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerLocateArchive

; Locate ZIP local file header structure
; Stack input:
;   near pointer to ZIP central directory end structure
;   near pointer to the file name
;   file name length
;   near pointer to output structure pointer
; Output:
;   AX - 0 when found, negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerSearchArchive

; Locate ZIP file data
; Stack input:
;   near pointer to ZIP local file header structure
;   near pointer to ZIP file data pointer
; Output:
;   AX - 0 when found, negative on error
; Invalidates: AX, BX, CX, DX
KER_API KerGetArchiveData


; ----- Error handling functions -----
API_SUBSET ERROR

; Print error message and terminate program execution
KER_API KerTerminate

; Byte variable for storing error code
KER_API KerLastError


%define KER_PIT_INPUT_FREQ              1193181_6667
%define KER_PIT_FREQ_DIVISOR            2048
%define KER_DELAY_MS_MULTIPLIER         100
%define KER_DELAY_MS_DIVISOR            ((1000_0000 * KER_DELAY_MS_MULTIPLIER) * KER_PIT_FREQ_DIVISOR / KER_PIT_INPUT_FREQ)

%define ERR_KER_UNSUPPORTED             ERR_CODE(ERR_FACILITY_KER, 1)
%define ERR_KER_NOT_FOUND               ERR_CODE(ERR_FACILITY_KER, 2)
%define ERR_KER_ARCHIVE_NOT_FOUND       ERR_CODE(ERR_FACILITY_KER, 3)
%define ERR_KER_ARCHIVE_TOO_LARGE       ERR_CODE(ERR_FACILITY_KER, 4)
%define ERR_KER_ARCHIVE_INVALID         ERR_CODE(ERR_FACILITY_KER, 5)
%define ERR_KER_INVALID_SEQUENCE        ERR_CODE(ERR_FACILITY_KER, 6)
