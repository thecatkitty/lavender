%ifndef CGA_API

%define CGA_API extern

; Copy bitmap to screen buffer
; Input:
;   DS:SI - linear monochrome bitmap
;   AH    - horizontal position, in octets
;   AL    - vertical position, in lines
;   CX    - image width
;   DX    - image height
; Invalidates: AX, CX, DX, DI, SI
CGA_API cga_draw_bitmap


; Write text to screen
; Input:
;   DS:SI - null-terminated ASCII string
;   AH    - horizontal position, in columns
;   AL    - vertical position, in rows
CGA_API cga_draw_text

; Set video mode
; Input:
;   AX - new video mode
; Output:
;   AX - previous video mode
; Invalidates: CX
CGA_API cga_set_video_mode

%endif

CGA_MODE_HIMONO     equ 6                         ; 640x200x1
CGA_HIMONO_MEM      equ 0B800h                    ; video memory base
CGA_HIMONO_LINE     equ (640 / 8)                 ; line size in bytes
CGA_HIMONO_PLANE    equ 2000h                     ; odd plane offset
