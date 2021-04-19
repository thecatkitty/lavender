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
                jne     .End

                ; Text - vertical position
                mov     al, [di + LINE.Vertical]

                ; Text - horizontal position
                cmp     word [di + LINE.Horizontal], 0FFF1h
                je      .AlignCenter
                cmp     word [di + LINE.Horizontal], 0FFF2h
                je      .AlignRight
                mov     ah, [di + LINE.Horizontal]
                jmp     .Write
.AlignCenter:
                mov     ah, LINE_MAX_LENGTH
                sub     ah, [di + LINE.Length]
                shr     ah, 1
                jmp     .Write
.AlignRight:
                mov     ah, LINE_MAX_LENGTH
                sub     ah, [di + LINE.Length]
  
.Write:
                mov     si, di
                add     si, LINE.Content
                call    VidDrawText

.End:           ret


; Halt execution for a given period
; Input:
;   CX - number of ticks (1 tick ~ 54,9 ms)
; Invalidates: CX
                global  Sleep
Sleep:
                jcxz    .End
.Next:
                hlt
                loop    .Next

.End:           ret
