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
  .Width:                       resw    1
  .Height:                      resw    1
  .WidthBytes:                  resw    1
  .Planes:                      resb    1
  .BitsPerPixel:                resb    1
  .Bits:                        resw    1
endstruc
