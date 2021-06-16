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

VID_MODE_CGA_HIMONO             equ     6               ; 640x200x1
VID_CGA_HIMONO_WIDTH            equ     640
VID_CGA_HIMONO_HEIGHT           equ     200
VID_CGA_HIMONO_LINE             equ     (VID_CGA_HIMONO_WIDTH / 8)

VID_EDID_MANUFACTURER_ID1       equ     10              ; .wManufacturerId
VID_EDID_MANUFACTURER_ID2       equ     5               ;   -111 1122  2223 3333
VID_EDID_MANUFACTURER_ID3       equ     0
VID_EDID_MANUFACTURER_IDx_MASK  equ     11111b

VID_EDID_INPUT_TYPE             equ     7               ; .bInputType (digital)
VID_EDID_INPUT_TYPE_ANALOG      equ     0               ;   TDDD IIII
VID_EDID_INPUT_TYPE_DIGITAL     equ     1
VID_EDID_INPUT_TYPE_MASK        equ     1b
VID_EDID_INPUT_DEPTH            equ     4
VID_EDID_INPUT_DEPTH_6B         equ     1
VID_EDID_INPUT_DEPTH_8B         equ     2
VID_EDID_INPUT_DEPTH_10B        equ     3
VID_EDID_INPUT_DEPTH_12B        equ     4
VID_EDID_INPUT_DEPTH_14B        equ     5
VID_EDID_INPUT_DEPTH_16B        equ     6
VID_EDID_INPUT_DEPTH_MASK       equ     111b
VID_EDID_INPUT_INTERFACE        equ     0
VID_EDID_INPUT_INTERFACE_HDMIA  equ     2
VID_EDID_INPUT_INTERFACE_HDMIB  equ     3
VID_EDID_INPUT_INTERFACE_MDDI   equ     4
VID_EDID_INPUT_INTERFACE_DP     equ     5
VID_EDID_INPUT_INTERFACE_MASK   equ     1111b

VID_EDID_INPUT_LEVELS           equ     5               ; .bInputType (analog)
VID_EDID_INPUT_LEVELS_0700_0300 equ     0               ;   -LLB SCGV
VID_EDID_INPUT_LEVELS_0714_0286 equ     1
VID_EDID_INPUT_LEVELS_1000_0400 equ     2
VID_EDID_INPUT_LEVELS_0700_0000 equ     3
VID_EDID_INPUT_LEVELS_MASK      equ     11b
VID_EDID_INPUT_BLANK_TO_BLACK   equ     4
VID_EDID_INPUT_SEPARATE_SYNC    equ     3
VID_EDID_INPUT_COMPOSITE_SYNC   equ     2
VID_EDID_INPUT_SYNC_ON_GREEN    equ     1
VID_EDID_INPUT_VSYNC_SERRATED   equ     0

VID_EDID_FEATURE_STANDBY        equ     7               ; .bFeatures (digital)
VID_EDID_FEATURE_SUSPEND        equ     6               ;   SUA2 4RDC
VID_EDID_FEATURE_ACTIVEOFF      equ     5
VID_EDID_FEATURE_YCRCB422       equ     4
VID_EDID_FEATURE_YCRCB444       equ     3
VID_EDID_FEATURE_TYPE           equ     3               ; .bFeatures (analog)
VID_EDID_FEATURE_TYPE_MONO      equ     0               ;   SUAT TRDC
VID_EDID_FEATURE_TYPE_RGB       equ     1
VID_EDID_FEATURE_TYPE_NONRGB    equ     2
VID_EDID_FEATURE_TYPE_MASK      equ     11b
VID_EDID_FEATURE_SRGB           equ     2
VID_EDID_FEATURE_PREFERRED_DT1  equ     1
VID_EDID_FEATURE_CONTINUOUS     equ     0

VID_EDID_TIMING_ASPECT          equ     14              ; .stStandardTiming
VID_EDID_TIMING_ASPECT_16_10    equ     0               ;   AAVV VVVV XXXX XXXX
VID_EDID_TIMING_ASPECT_4_3      equ     1
VID_EDID_TIMING_ASPECT_5_4      equ     2
VID_EDID_TIMING_ASPECT_16_9     equ     3
VID_EDID_TIMING_ASPECT_MASK     equ     11b
VID_EDID_TIMING_VFREQ           equ     8
VID_EDID_TIMING_VFREQ_MASK      equ     111111b
VID_EDID_TIMING_XRES            equ     0
VID_EDID_TIMING_XRES_MASK       equ     11111111b

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

