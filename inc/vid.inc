%include "err.inc"

%define VID_API extern


; ----- Display mode functions -----

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


%define VID_MODE_CGA_HIMONO             6               ; 640x200x1

%define ERR_VID_UNSUPPORTED             ERR_CODE(ERR_FACILITY_VID, 0)
%define ERR_VID_FAILED                  ERR_CODE(ERR_FACILITY_VID, 1)
%define ERR_VID_FORMAT                  ERR_CODE(ERR_FACILITY_VID, 2)
