%include "err.inc"

%ifndef VID_API

%define VID_API extern

; ----- Low level drawing functions -----

; Copy bitmap to screen buffer
; Stack input:
;   near pointer to GFX_BITMAP structure
;   horizontal position, in columns (aligned on 8)
;   vertical position, in lines
; Output:
;   AX - negative on error
; Invalidates: AX, BX, CX, DX
VID_API VidDrawBitmap

; Write text to screen
; Stack input:
;   near pointer to a null-terminated ASCII string
;   horizontal position, in columns
;   vertical position, in lines
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
; Stack input:
;   Unicode code point
; Output:
;   AL - local encoding code point, '?' on error
; Invalidates: AX, BX, CX, DX
VID_API VidConvertToLocal

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
