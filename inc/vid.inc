%include "err.inc"

%ifndef VID_API

%define VID_API extern

; ----- Low level drawing functions -----

; Copy bitmap to screen buffer
; Input:
;   DS:SI - GFX_BITMAP structure
;   AX    - horizontal position, in columns (aligned on 8)
;   BX    - vertical position, in lines
; Invalidates: AX, CX, DX, DI, SI
VID_API VidDrawBitmap

; Write text to screen
; Input:
;   DS:SI - null-terminated ASCII string
;   AH    - horizontal position, in columns
;   AL    - vertical position, in rows
VID_API VidDrawText

; ----- Display mode functions -----

; Get pixel aspect ratio
; Output:
;   AX - pixel aspect ratio (PAR = 64 / AL), default value on error
; Invalidates: AX, BX, CX, DX
VID_API VidGetPixelAspectRatio

; Set video mode
; Stack input:
;   new video mode
; Output:
;   AX - previous video mode
; Invalidates: AX
VID_API VidSetMode

; ----- Font functions -----

; Initialize extended screen font
VID_API VidLoadFont

; Deinitialize extended screen font
VID_API VidUnloadFont

; Get local encoding code point from Unicode code point
; Input:
;   AX - Unicode code point
; Output:
;   AL - local encoding code point, '?' on error
;   CF - error
VID_API VidGetFontEncoding

%endif

%define VID_MODE_CGA_HIMONO             6               ; 640x200x1
%define VID_CGA_HIMONO_WIDTH            640
%define VID_CGA_HIMONO_HEIGHT           200
%define VID_CGA_HIMONO_LINE             (VID_CGA_HIMONO_WIDTH / 8)

%define ERR_VID_UNSUPPORTED             ERR_CODE(ERR_FACILITY_VID, 0)
%define ERR_VID_FAILED                  ERR_CODE(ERR_FACILITY_VID, 1)
%define ERR_VID_FORMAT                  ERR_CODE(ERR_FACILITY_VID, 2)

struc VID_CHARACTER_DESCRIPTOR
  .wcCodePoint:                 resw    1
  .pabOverlay:                  resw    1
  .cBase:                       resb    1
endstruc
