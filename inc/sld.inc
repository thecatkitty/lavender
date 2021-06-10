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

SLD_DELAY_MS_MULTIPLIER         equ     10
SLD_DELAY_MS_DIVISOR            equ     (1000_0000 * SLD_DELAY_MS_MULTIPLIER) \
                                        / 18_2065

SLD_TYPE_TEXT                   equ     0
SLD_TYPE_BITMAP                 equ     1

SLD_ALIGN_LEFT                  equ     0
SLD_ALIGN_CENTER                equ     0FFF1h
SLD_ALIGN_RIGHT                 equ     0FFF2h

struc SLD_ENTRY
  .Delay:                       resw    1
  .Horizontal:                  resw    1
  .Vertical:                    resw    1
  .Type:                        resb    1
  .Content:                     resb    SLD_ENTRY_MAX_LENGTH + 1
  .Length:                      resb    1
endstruc
