%ifndef GFX_API

%define GFX_API extern

; Load bitmap picture
; Input:
;   DS:SI - picture data
;   DS:DI - GFX_BITMAP structure
; Output:
;   CF    - error
GFX_API GfxLoadBitmap

%endif

GFX_PBM_RAW_MAGIC               equ     03450h          ; P4

struc GFX_BITMAP
  .wWidth:                      resw    1
  .wHeight:                     resw    1
  .wWidthBytes:                 resw    1
  .bPlanes:                     resb    1
  .bBitsPerPixel:               resb    1
  .pBits:                       resw    1
endstruc
