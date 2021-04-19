%include "vid.inc"
%include "line.inc"

cpu 8086

LOGOW equ 272                 ; logo width
LOGOH equ 100                 ; logo height

extern _ext_start

[bits 16]
section .init
  jmp  start

section .text

start:
  ; Set CGA video mode
  mov  ax, VID_MODE_CGA_HIMONO; 640x200x2
  call VidSetMode
  push ax                     ; save previous mode on stack

  mov  si, bitmap
  mov  ah, (640 - LOGOW) / 2 / 8
  mov  al, (144 - LOGOH) / 2
  mov  cx, LOGOW
  mov  dx, LOGOH
  call VidDrawBitmap

  mov  si, _ext_start
.next:
  mov  di, oLine
  call LineLoad
  jc   .error
  test ax, ax
  jz   .end
  push si

  call LineExecute
  pop  si
  jmp .next

.end:
  xor  ah, ah                 ; GET KEYSTROKE
  int  16h

  pop  ax                     ; restore saved mode
  call VidSetMode

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


section .data
  bitmap            incbin "../data/cgihisym.pbm", 57
  msg_err           db     "ERR$"


section .bss
  oLine             resb   LINE_size
