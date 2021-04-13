org 100h

LOGOW equ 272                 ; logo width
LOGOH equ 100                 ; logo height

section .text

start:
  mov  ax, CGA_MODE_HIMONO    ; 640x200x2
  call cga_set_video_mode
  push ax                     ; save previous mode on stack

  mov  si, bitmap
  mov  cx, LOGOW
  mov  dx, LOGOH
  call cga_draw_bitmap

  xor  ah, ah                 ; GET KEYSTROKE
  int  16h

  pop  ax                     ; restore saved mode
  call cga_set_video_mode
  
  mov  ax, 4C00h              ; TERMINATE WITH RETURN CODE -> 0
  int  21h

%include "cga.asm"

section .data
  bitmap            incbin "cgihisym.pbm", 57

section .bss
  