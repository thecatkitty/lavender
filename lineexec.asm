section .text

line_execute:
  ; Delay
  mov  cx, [line_delay]
  call sleep

  ; Type
  cmp  byte [line_type], 0
  jne  .end

  ; Text - vertical position
  mov  al, [line_y]

  ; Text - horizontal position
  cmp  word [line_x], 0FFF1h
  je   .center
  cmp  word [line_x], 0FFF2h
  je   .right
  mov  ah, [line_x]
  jmp  .write

.center:
  mov  ah, LINE
  sub  ah, [line_length]
  shr  ah, 1
  jmp  .write

.right:
  mov  ah, LINE
  sub  ah, [line_length]
  
.write:
  mov  si, line_content
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


%include "lineread.asm"


section .bss
  fslides           resw   1
  buff              resw   LINE

  line_delay        resw   1
  line_x            resw   1
  line_y            resw   1
  line_type         resb   1
  line_content      resb   LINE + 1
  line_length       resb   1
