org 100h
cpu 8086

LOGOW equ 272                 ; logo width
LOGOH equ 100                 ; logo height

section .text

start:
  mov  ax, CGA_MODE_HIMONO    ; 640x200x2
  call cga_set_video_mode
  push ax                     ; save previous mode on stack

  mov  si, bitmap
  mov  ah, (640 - LOGOW) / 2 / 8
  mov  al, (144 - LOGOH) / 2
  mov  cx, LOGOW
  mov  dx, LOGOH
  call cga_draw_bitmap

  mov  si, msg_bronies
  mov  ah, 32
  mov  al, 18
  call cga_draw_text

  mov  si, msg_version
  mov  ah, 35
  mov  al, 19
  call cga_draw_text

  mov  si, msg_copyright
  mov  ah, 10
  mov  al, 23
  call cga_draw_text

  mov  si, msg_press
  mov  ah, 24
  mov  al, 24
  call cga_draw_text

  xor  ah, ah                 ; GET KEYSTROKE
  int  16h

  pop  ax                     ; restore saved mode
  call cga_set_video_mode
  
  mov  ax, 4C00h              ; TERMINATE WITH RETURN CODE -> 0
  int  21h

%include "cga.asm"

section .data
  bitmap            incbin "cgihisym.pbm", 57
  msg_bronies       db     "Bronies Twilight", 0
  msg_version       db     "Wersja 2.0", 0
  msg_copyright     db     "Copyright (c) Fundacja BT, 2021. Wszelkie prawa zastrzezone.", 0
  msg_press         db     "Nacisnij ENTER, aby kontynuowac.", 0

section .bss
  