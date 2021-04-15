org 100h
cpu 8086

LOGOW equ 272                 ; logo width
LOGOH equ 100                 ; logo height
LINE  equ 80                  ; max line length

%define milliseconds(ms) (ms * 10 / 549)

section .text

start:
  ; Set CGA video mode
  mov  ax, CGA_MODE_HIMONO    ; 640x200x2
  call cga_set_video_mode
  push ax                     ; save previous mode on stack

  ; Open slideshow file
  mov  ax, 3D00h              ; OPEN EXISTING FILE -> read only
  mov  dx, slides
  int  21h
  jc   .error
  mov  [fslides], ax

  mov  si, bitmap
  mov  ah, (640 - LOGOW) / 2 / 8
  mov  al, (144 - LOGOH) / 2
  mov  cx, LOGOW
  mov  dx, LOGOH
  call cga_draw_bitmap

.next:
  call line_read
  jc   .error
  test ax, ax
  jz   .eof
  push si
  
  call line_execute
  pop  si
  jmp .next

.eof:
  ; Close file
  mov  ah, 3Eh
  int  21h
  jmp  .end

.end:
  xor  ah, ah                 ; GET KEYSTROKE
  int  16h

  pop  ax                     ; restore saved mode
  call cga_set_video_mode

  mov  ax, 4C00h              ; TERMINATE WITH RETURN CODE -> 0
  int  21h

.error:
  push ax
  mov  ah, 09h                ; WRITE STRING TO STANDARD OUTPUT
  mov  dx, msg_err
  int  21h

  pop  ax
  mov  ah, 4Ch                ; TERMINATE WITH RETURN CODE -> AL
  int  21h


%include "cga.asm"
%include "lineexec.asm"

section .data
  bitmap            incbin "cgihisym.pbm", 57
  slides            db     "slides.txt", 0
  msg_err           db     "ERR$"

section .bss
