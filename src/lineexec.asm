%define LINE_API
%include "line.inc"
%include "vid.inc"


                cpu     8086

[bits 16]
section .text

                global  LineExecute
LineExecute:
                ; Delay
                mov     cx, [di + LINE.Delay]
                call    Sleep

                ; Type
                cmp     byte [di + LINE.Type], 0
                jne     .end

                ; Text - vertical position
                mov     al, [di + LINE.Vertical]

                ; Text - horizontal position
                cmp     word [di + LINE.Horizontal], 0FFF1h
                je      .center
                cmp     word [di + LINE.Horizontal], 0FFF2h
                je      .right
                mov     ah, [di + LINE.Horizontal]
                jmp     .write
.center:
                mov     ah, LINE_MAX_LENGTH
                sub     ah, [di + LINE.Length]
                shr     ah, 1
                jmp     .write
.right:
                mov     ah, LINE_MAX_LENGTH
                sub     ah, [di + LINE.Length]
  
.write:
                mov     si, di
                add     si, LINE.Content
                call    VidDrawText

.end:           ret


; Halt execution for a given period
; Input:
;   CX - number of ticks (1 tick ~ 54,9 ms)
; Invalidates: CX
                global  Sleep
Sleep:
                jcxz    .end
.next:
                hlt
                loop    .next

.end:           ret
