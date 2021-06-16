%include "err.inc"

%ifndef SLD_API

%define SLD_API extern

; Load the next line from the slideshow file
; Input:
;   DS:SI - slideshow file line
;   DS:DI - SLD_ENTRY structure
; Output:
;   DS:SI - next slideshow file line
;   AX    - line content length
;   CF    - error
SLD_API SldEntryLoad

; Execute a line loaded from the slideshow file
; Input:
;   DS:SI - ZIP central directory end structure
;   DS:DI - SLD_ENTRY structure
; Output:
;   CF    - error
SLD_API SldEntryExecute

%endif

SLD_ENTRY_MAX_LENGTH            equ     80

SLD_TAG_TYPE_TEXT               equ     'T'
SLD_TAG_TYPE_BITMAP             equ     'B'
SLD_TAG_ALIGN_LEFT              equ     '<'
SLD_TAG_ALIGN_CENTER            equ     '^'
SLD_TAG_ALIGN_RIGHT             equ     '>'

SLD_TYPE_TEXT                   equ     0
SLD_TYPE_BITMAP                 equ     1

SLD_ALIGN_LEFT                  equ     0
SLD_ALIGN_CENTER                equ     0FFF1h
SLD_ALIGN_RIGHT                 equ     0FFF2h

%define ERR_SLD_INVALID_DELAY           ERR_CODE(ERR_FACILITY_SLD, 0)
%define ERR_SLD_UNKNOWN_TYPE            ERR_CODE(ERR_FACILITY_SLD, 1)
%define ERR_SLD_INVALID_VERTICAL        ERR_CODE(ERR_FACILITY_SLD, 2)
%define ERR_SLD_INVALID_HORIZONTAL      ERR_CODE(ERR_FACILITY_SLD, 3)

struc SLD_ENTRY
  .wDelay:                      resw    1
  .wHorizontal:                 resw    1
  .wVertical:                   resw    1
  .bType:                       resb    1
  .szContent:                   resb    SLD_ENTRY_MAX_LENGTH + 1
  .bLength:                     resb    1
endstruc
