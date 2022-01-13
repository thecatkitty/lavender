%include "err.inc"

%ifndef SLD_API

%define SLD_API extern

; Load the next line from the slideshow file
; Stack input:
;   near pointer to slideshow file line
;   near pointer to SLD_ENTRY structure
; Output:
;   AX    - total line length in bytes, negative on error
; Invalidates: AX, BX, CX, DX
SLD_API SldLoadEntry

; Execute a line loaded from the slideshow file
; Input:
;   DS:SI - ZIP central directory end structure
;   DS:DI - SLD_ENTRY structure
; Output:
;   CF    - error
SLD_API SldEntryExecute

%endif

%define SLD_ENTRY_MAX_LENGTH            80

%define SLD_TAG_TYPE_TEXT               'T'
%define SLD_TAG_TYPE_BITMAP             'B'
%define SLD_TAG_ALIGN_LEFT              '<'
%define SLD_TAG_ALIGN_CENTER            '^'
%define SLD_TAG_ALIGN_RIGHT             '>'

%define SLD_TYPE_TEXT                   0
%define SLD_TYPE_BITMAP                 1

%define SLD_ALIGN_LEFT                  0
%define SLD_ALIGN_CENTER                0FFF1h
%define SLD_ALIGN_RIGHT                 0FFF2h

%define ERR_SLD_INVALID_DELAY           ERR_CODE(ERR_FACILITY_SLD, 0)
%define ERR_SLD_UNKNOWN_TYPE            ERR_CODE(ERR_FACILITY_SLD, 1)
%define ERR_SLD_INVALID_VERTICAL        ERR_CODE(ERR_FACILITY_SLD, 2)
%define ERR_SLD_INVALID_HORIZONTAL      ERR_CODE(ERR_FACILITY_SLD, 3)
%define ERR_SLD_CONTENT_TOO_LONG        ERR_CODE(ERR_FACILITY_SLD, 4)

struc SLD_ENTRY
  .wDelay:                      resw    1
  .wHorizontal:                 resw    1
  .wVertical:                   resw    1
  .bType:                       resb    1
  .szContent:                   resb    SLD_ENTRY_MAX_LENGTH + 1
  .bLength:                     resb    1
endstruc
