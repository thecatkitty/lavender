%ifndef PIC_API

%define PIC_API extern

; Load bitmap picture
; Input:
;   DS:SI - picture data
;   DS:DI - PIC_BITMAP structure
; Output:
;   CF    - error
PIC_API PicLoadBitmap

%endif

PIC_PBM_RAW_MAGIC               equ     03450h          ; P4

struc PIC_BITMAP
  .wWidth:                      resw    1
  .wHeight:                     resw    1
  .wWidthBytes:                 resw    1
  .bPlanes:                     resb    1
  .bBitsPerPixel:               resb    1
  .pBits:                       resw    1
endstruc
