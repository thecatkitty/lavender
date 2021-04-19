%define LINE_API
%include "line.inc"
%include "cga.inc"

cpu 8086

global line_execute

[bits 16]
section .text

line_execute:
  ; Delay
  mov  cx, [di + LINE.Delay]
  call sleep

  ; Type
  cmp  byte [di + LINE.Type], 0
  jne  .end

  ; Text - vertical position
  mov  al, [di + LINE.Vertical]

  ; Text - horizontal position
  cmp  word [di + LINE.Horizontal], 0FFF1h
  je   .center
  cmp  word [di + LINE.Horizontal], 0FFF2h
  je   .right
  mov  ah, [di + LINE.Horizontal]
  jmp  .write

.center:
  mov  ah, LINE_MAX_LENGTH
  sub  ah, [di + LINE.Length]
  shr  ah, 1
  jmp  .write

.right:
  mov  ah, LINE_MAX_LENGTH
  sub  ah, [di + LINE.Length]
  
.write:
  mov  si, di
  add  si, LINE.Content
  call cga_draw_text

.end:
  ret
  
; Halt execution for a given period
; Input:
;   CX - number of ticks (1 tick ~ 54,9 ms)
; Invalidates: CX
sleep:
  jcxz .end
  hlt
  loop sleep
.end:
  ret
