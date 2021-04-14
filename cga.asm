CGA_MODE_HIMONO     equ 6                         ; 640x200x1
CGA_HIMONO_MEM      equ 0B800h                    ; video memory base
CGA_HIMONO_LINE     equ (640 / 8)                 ; line size in bytes
CGA_HIMONO_PLANE    equ 2000h                     ; odd plane offset

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
; Invalidates: AX, CX, DX, DI, SI
cga_draw_bitmap:
  mov  di, ax                 ; get the offset
  mov  ah, al                 
  mov  al, CGA_HIMONO_LINE / 2
  mul  ah                     ; AX = vertical position * CGA_HIMONO_LINE / 2
  shr  di, 8                  ; DI = horizontal position
  add  di, ax

  shr  cx, 3                  ; get the image width in octets

  push es                     ; save and set the segment register
  mov  ax, CGA_HIMONO_MEM     
  mov  es, ax
.next:
  call cga_draw_line          ; draw even line
  xor  di, CGA_HIMONO_PLANE
  dec  dx
  
  call cga_draw_line          ; draw odd line
  add  di, CGA_HIMONO_LINE
  xor  di, CGA_HIMONO_PLANE
  dec  dx
  jnz  .next

  pop  es                     ; restore the segment register
  ret


; Copy bitmap line to screen buffer
; Input:
;   DS:SI - bitmap line
;   ES:DI - screen line
;   CX    - number of octets
; Output:
;   DS:SI - next bitmap line
; Invalidates: AX
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
  pop  di
  ret
