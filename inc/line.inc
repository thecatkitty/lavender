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

LINE_MAX_LENGTH     equ    80

struc LINE
  .Delay:           resw   1
  .Horizontal:      resw   1
  .Vertical:        resw   1
  .Type:            resb   1
  .Content:         resb   LINE_MAX_LENGTH + 1
  .Length:          resb   1
endstruc
