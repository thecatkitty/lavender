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
;   AL - pixel aspect ratio (PAR = 64 / AL), default value on error
;   CF - error
; Invalidates: AX
VID_API VidGetPixelAspectRatio

; Set video mode
; Input:
;   AX - new video mode
; Output:
;   AX - previous video mode
; Invalidates: CX
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

%define VID_EDID_MANUFACTURER_ID1       10              ; .wManufacturerId
%define VID_EDID_MANUFACTURER_ID2       5               ;   -111 1122  2223 3333
%define VID_EDID_MANUFACTURER_ID3       0
%define VID_EDID_MANUFACTURER_IDx_MASK  11111b

%define VID_EDID_INPUT_TYPE             7               ; .bInputType (digital)
%define VID_EDID_INPUT_TYPE_ANALOG      0               ;   TDDD IIII
%define VID_EDID_INPUT_TYPE_DIGITAL     1
%define VID_EDID_INPUT_TYPE_MASK        1b
%define VID_EDID_INPUT_DEPTH            4
%define VID_EDID_INPUT_DEPTH_6B         1
%define VID_EDID_INPUT_DEPTH_8B         2
%define VID_EDID_INPUT_DEPTH_10B        3
%define VID_EDID_INPUT_DEPTH_12B        4
%define VID_EDID_INPUT_DEPTH_14B        5
%define VID_EDID_INPUT_DEPTH_16B        6
%define VID_EDID_INPUT_DEPTH_MASK       111b
%define VID_EDID_INPUT_INTERFACE        0
%define VID_EDID_INPUT_INTERFACE_HDMIA  2
%define VID_EDID_INPUT_INTERFACE_HDMIB  3
%define VID_EDID_INPUT_INTERFACE_MDDI   4
%define VID_EDID_INPUT_INTERFACE_DP     5
%define VID_EDID_INPUT_INTERFACE_MASK   1111b

%define VID_EDID_INPUT_LEVELS           5               ; .bInputType (analog)
%define VID_EDID_INPUT_LEVELS_0700_0300 0               ;   -LLB SCGV
%define VID_EDID_INPUT_LEVELS_0714_0286 1
%define VID_EDID_INPUT_LEVELS_1000_0400 2
%define VID_EDID_INPUT_LEVELS_0700_0000 3
%define VID_EDID_INPUT_LEVELS_MASK      11b
%define VID_EDID_INPUT_BLANK_TO_BLACK   4
%define VID_EDID_INPUT_SEPARATE_SYNC    3
%define VID_EDID_INPUT_COMPOSITE_SYNC   2
%define VID_EDID_INPUT_SYNC_ON_GREEN    1
%define VID_EDID_INPUT_VSYNC_SERRATED   0

%define VID_EDID_FEATURE_STANDBY        7               ; .bFeatures (digital)
%define VID_EDID_FEATURE_SUSPEND        6               ;   SUA2 4RDC
%define VID_EDID_FEATURE_ACTIVEOFF      5
%define VID_EDID_FEATURE_YCRCB422       4
%define VID_EDID_FEATURE_YCRCB444       3
%define VID_EDID_FEATURE_TYPE           3               ; .bFeatures (analog)
%define VID_EDID_FEATURE_TYPE_MONO      0               ;   SUAT TRDC
%define VID_EDID_FEATURE_TYPE_RGB       1
%define VID_EDID_FEATURE_TYPE_NONRGB    2
%define VID_EDID_FEATURE_TYPE_MASK      11b
%define VID_EDID_FEATURE_SRGB           2
%define VID_EDID_FEATURE_PREFERRED_DT1  1
%define VID_EDID_FEATURE_CONTINUOUS     0

%define VID_EDID_TIMING_ASPECT          14              ; .stStandardTiming
%define VID_EDID_TIMING_ASPECT_16_10    0               ;   AAVV VVVV XXXX XXXX
%define VID_EDID_TIMING_ASPECT_4_3      1
%define VID_EDID_TIMING_ASPECT_5_4      2
%define VID_EDID_TIMING_ASPECT_16_9     3
%define VID_EDID_TIMING_ASPECT_MASK     11b
%define VID_EDID_TIMING_VFREQ           8
%define VID_EDID_TIMING_VFREQ_MASK      111111b
%define VID_EDID_TIMING_XRES            0
%define VID_EDID_TIMING_XRES_MASK       11111111b

%define VID_PAR(dx,dy,sx,sy)            (64 * dy * sx / dx / sy)

%define ERR_VID_UNSUPPORTED             ERR_CODE(ERR_FACILITY_VID, 0)
%define ERR_VID_FAILED                  ERR_CODE(ERR_FACILITY_VID, 1)
%define ERR_VID_FORMAT                  ERR_CODE(ERR_FACILITY_VID, 2)

struc VID_EDID
  ; Header information
  .abPadding:                   resb    8
  .wManufacturerId:             resw    1
  .wModelId:                    resw    1
  .dwSerialNumber:              resd    1
  .bWeek:                       resb    1
  .bYear:                       resb    1
  .bEdidVersion:                resb    1
  .bEdidRevision:               resb    1
  ; Display parameters
  .bInputType:                  resb    1
  .bHorizontalSize:             resb    1
  .bVerticalSize:               resb    1
  .bGamma:                      resb    1
  .bFeatures:                   resb    1
  .stChroma:                    resb    10
  .abEstablishedModes:          resb    3
  ; Standard timing information
  .stStandardTiming:            resw    8
  .stDetailedTiming1:           resb    18 
  .stDetailedTiming2:           resb    18 
  .stDetailedTiming3:           resb    18 
  .stDetailedTiming4:           resb    18 
  ; Footer
  .bExtensions:                 resb    1
  .bChecksum:                   resb    1
endstruc

struc VID_CHARACTER_DESCRIPTOR
  .wcCodePoint:                 resw    1
  .pabOverlay:                  resw    1
  .cBase:                       resb    1
endstruc

