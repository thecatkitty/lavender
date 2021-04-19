%define CGA_API
%include "vid.inc"

global VidDrawBitmap
global VidDrawText
global VidSetMode

[bits 16]
section .text

VidSetMode:
  push ax
  mov  ah, 0Fh                ; GET CURRENT VIDEO MODE
  int  10h
  xor  ah, ah                 ; store previous video mode in CX
  mov  cx, ax

  pop  ax
  int  10h                    ; SET VIDEO MODE

  mov  ax, cx                 ; return previous video mode
  ret


VidDrawBitmap:
  push dx                     ; save image height
  push cx                     ; save image width

  mov  di, ax
  mov  cl, 8
  shr  di, cl                 ; DI = horizontal position
  mov  ah, al                 
  mov  al, VID_CGA_HIMONO_LINE / 2
  mul  ah                     ; AX = vertical position * VID_CGA_HIMONO_LINE / 2
  add  di, ax                 ; get the offset

  pop  dx                     ; restore image width
  mov  cl, 3
  shr  dx, cl                 ; get the image width in octets
  mov  cx, dx
  pop  dx                     ; restore image height

  push es                     ; save and set the segment register
  mov  ax, VID_CGA_HIMONO_MEM     
  mov  es, ax
.next:
  call cga_draw_line          ; draw even line
  xor  di, VID_CGA_HIMONO_PLANE
  dec  dx
  
  call cga_draw_line          ; draw odd line
  add  di, VID_CGA_HIMONO_LINE
  xor  di, VID_CGA_HIMONO_PLANE
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
cga_draw_line:
  push di
  push cx
  cld
  rep  movsb
  pop  cx
  pop  di
  ret


VidDrawText:
  push dx                     ; save registers
  push bx
  push ax

  mov  dh, al
  mov  dl, ah
  xor  bx, bx
  mov  ah, 02h                ; SET CURSOR POSITION
  int  10h

  mov  ah, 02h                ; WRITE CHARACTER TO STANDARD OUTPUT
.next:
  mov  dl, [si]
  test dl, dl
  jz   .end
  int  21h
  inc  si
  jmp  .next

.end:
  pop  ax
  pop  bx
  pop  cx
  ret
