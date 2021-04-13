CGA_MODE_HIMONO     equ 6                         ; 640x200x1
CGA_HIMONO_FB_EVEN  equ 0B800h                    ; even lines
CGA_HIMONO_FB_ODD   equ 0BA00h                    ; odd lines
CGA_HIMONO_LINE     equ (640 / 8)                 ; line size in bytes

section .text

; Set video mode
; Input:
;   AX - new video mode
; Output:
;   AX - previous video mode
; Invalidates: CX
cga_set_video_mode:
  push ax
  mov  ah, 0Fh                ; GET CURRENT VIDEO MODE
  int  10h
  xor  ah, ah                 ; store previous video mode in CX
  mov  cx, ax

  pop  ax
  int  10h                    ; SET VIDEO MODE

  mov  ax, cx                 ; return previous video mode
  ret


; Copy bitmap to screen buffer
; Input:
;   DS:SI - linear monochrome bitmap
;   AH    - horizontal position, in octets
;   AL    - vertical position, in lines
;   CX    - image width
;   DX    - image height
; Invalidates: DI
cga_draw_bitmap:
  push es                     ; save the segment register

  mov  di, ax                 ; get the offset
  mov  ah, al                 
  mov  al, CGA_HIMONO_LINE / 2
  mul  ah                     ; AX = vertical position * CGA_HIMONO_LINE / 2
  shr  di, 8                  ; DI = horizontal position
  add  di, ax
  push di                     ; save the offset
  shr  cx, 3                  ; get the image width in octets
  push cx                     ; save the image width in octets
  shr  dx, 1                  ; get the image half-height, copy every other line
  push dx                     ; save the image half-height
  push si                     ; save the address of the first line of the bitmap

  mov  ax, CGA_HIMONO_FB_EVEN ; write even lines
  mov  es, ax
.even:
  call cga_draw_line
  dec  dx
  jnz  .even

  pop  si                     ; restore the address of the first line
  pop  dx                     ; restore the image half-height, copy every other line
  pop  cx                     ; restore the number of horizontal octets
  add  si, cx                 ; start from the second line of the bitmap

  mov  ax, CGA_HIMONO_FB_ODD  ; write odd lines
  mov  es, ax
  pop  di                     ; restore the offset
.odd:
  call cga_draw_line
  dec  dx
  jnz  .odd

  pop  es                     ; restore the segment register
  ret


; Copy bitmap line to screen buffer
; Input:
;   DS:SI - bitmap line
;   ES:DI - screen line
;   CX    - number of octets
; Output:
;   DS:SI - next bitmap line
;   ES:DI - next screen line
; Invalidates: AL
cga_draw_line:
  push di
  push cx
.octet:
  mov  al, [si]
  mov  [es:di], al
  inc  si
  inc  di
  loop .octet

  pop  cx
  add  si, cx
  pop  di
  add  di, CGA_HIMONO_LINE
  ret
