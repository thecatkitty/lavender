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
;   ZF - 0 if DOSBox
; Invalidates: CX
KER_API KerIsDosBox


; ----- Time related functions -----
API_SUBSET TIME

; Halt execution for a given period
; Input:
;   CX - number of ticks (1 tick ~ 54,9 ms)
; Invalidates: CX
KER_API KerSleep

; ----- Interrupt related functions -----
API_SUBSET INT

; Install interrupt service routine
; Input:
;   ES:SI - interrupt service routine
;   DI    - interrupt number
;   ES:BX - output pointer
; Output:
;   ES:BX - long pointer to previous interrupt service routine
; Invalidates: DI
KER_API KerInstallIsr

; Uninstall interrupt service routine
; Input:
;   ES:SI - long pointer to previous interrupt service routine
;   DI    - interrupt number
; Invalidates: DI
KER_API KerUninstallIsr


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

; Compare two memory ranges
; Input:
;   DS:SI - first range
;   DS:BX - second range
;   CX    - range length
; Output:
;   CF, ZF - difference
KER_API KerCompareMemory

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
; Input:
;   DS:SI - UTF-8 sequence
; Output:
;   AX    - code point under 0800h
;   DS:SI - first byte after the end of sequence
;   CF    - error
KER_API KerGetCharacterFromUtf8

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
KER_API KerCompareUtf8IgnoreCase

; Get case folding for Unicode code point
;   CURRENTLY WORKS ONLY FOR CODE POINTS UNDER 0800h
; Input:
;   AX    - code point
; Output:
;   AX    - case folding first code point
;   BX    - case folding second code point or NUL
KER_API KerFoldCase


; ----- ZIP archive functions -----
API_SUBSET ZIP

; Locate ZIP central directory end structure
; Input:
;   DS:BX - search range lower boundary
;   DS:SI - search range upper boundary
; Output:
;   DS:SI - ZIP central directory end structure
;   CF    - error
; Invalidates:
KER_API KerLocateArchive

; Locate ZIP local file header structure
; Input:
;   DS:SI - ZIP central directory end structure
;   DS:BX - file name
;   CX    - file name length
; Output:
;   DS:DI - ZIP local file header structure
;   CF    - error
; Invalidates: BX, CX
KER_API KerSearchArchive

; Locate ZIP file data
; Input:
;   DS:DI - ZIP local file header structure
; Output:
;   DS:BX - ZIP file data
;   CF    - error
KER_API KerGetArchiveData


; ----- Error handling functions -----
API_SUBSET ERROR

; Print error message and terminate program execution
KER_API KerTerminate

; Byte variable for storing error code
KER_API KerLastError


KER_PIT_INPUT_FREQ              equ     1193181_6667
KER_PIT_FREQ_DIVISOR            equ     2048
KER_DELAY_MS_MULTIPLIER         equ     100
KER_DELAY_MS_DIVISOR            equ     (1000_0000 * KER_DELAY_MS_MULTIPLIER) \
                                        * KER_PIT_FREQ_DIVISOR / KER_PIT_INPUT_FREQ
