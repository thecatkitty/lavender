%ifndef LINE_API

%define LINE_API extern

; Load the next line from the slideshow file 
; Input:
;   DS:DI - LINE structure
;   BX    - file handle
; Output:
;   AX    - line content length
;   AX=0  - end of file
;   CF    - error
LINE_API line_read

; Load the next line from the slideshow file 
; Input:
;   DS:DI - LINE structure
LINE_API line_execute

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
