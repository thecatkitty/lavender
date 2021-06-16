%include "err.inc"

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

%define ERR_GFX_FORMAT                  ERR_CODE(ERR_FACILITY_GFX, 0)
%define ERR_GFX_MALFORMED_FILE          ERR_CODE(ERR_FACILITY_GFX, 1)

struc GFX_BITMAP
  .wWidth:                      resw    1
  .wHeight:                     resw    1
  .wWidthBytes:                 resw    1
  .bPlanes:                     resb    1
  .bBitsPerPixel:               resb    1
  .pBits:                       resw    1
endstruc
