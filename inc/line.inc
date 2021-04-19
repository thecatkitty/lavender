%ifndef LINE_API

%define LINE_API extern

; Load the next line from the slideshow file
; Input:
;   DS:SI - slideshow file line
;   DS:DI - LINE structure
; Output:
;   DS:SI - next slideshow file line
;   AX    - line content length
;   CF    - error
LINE_API LineLoad

; Execute a line loaded from the slideshow file
; Input:
;   DS:DI - LINE structure
LINE_API LineExecute

%endif

LINE_MAX_LENGTH                 equ     80

LINE_TAG_TYPE_TEXT              equ     'T'
LINE_TAG_TYPE_BITMAP            equ     'B'
LINE_TAG_ALIGN_LEFT             equ     '<'
LINE_TAG_ALIGN_CENTER           equ     '^'
LINE_TAG_ALIGN_RIGHT            equ     '>'

LINE_DELAY_MS_MULTIPLIER        equ     10
LINE_DELAY_MS_DIVISOR           equ     (1000_0000 * LINE_DELAY_MS_MULTIPLIER) \
                                        / 18_2065

LINE_TYPE_TEXT                  equ     0
LINE_TYPE_BITMAP                equ     1

LINE_ALIGN_LEFT                 equ     0
LINE_ALIGN_CENTER               equ     0FFF1h
LINE_ALIGN_RIGHT                equ     0FFF2h

struc LINE
  .Delay:                       resw    1
  .Horizontal:                  resw    1
  .Vertical:                    resw    1
  .Type:                        resb    1
  .Content:                     resb    LINE_MAX_LENGTH + 1
  .Length:                      resb    1
endstruc
