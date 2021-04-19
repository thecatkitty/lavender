%define STR_API
%include "str.inc"


                cpu     8086

[bits 16]
section .text


                global  StrParseU16
StrParseU16:
                push    bx              ; save registers
                push    dx

                xor     ax, ax          ; sum
                xor     bx, bx          ; character
                xor     cx, cx          ; count

.LeftPad:
                mov     bl, [si]
                cmp     bl, ' '
                jne     .Digit

                inc     cx
                inc     si
                jmp    .LeftPad

.Digit:
                mov     bl, [si]
                cmp     bl, '0'
                jb      .BelowZero
                cmp     bl, '9'
                ja      .Error          ; invalid character

                mov     dx, 10          ; multiplier
                mul     dx
                sub     bl, '0'
                add     ax, bx          ; AX = (AX * 10) mod 65536 + BL

                inc     cx
                inc     si
                jmp    .Digit

.BelowZero:
                cmp     bl, ' '
                jne     .Error          ; invalid character

                inc     cx
                inc     si

.RightPad:
                mov     bl, [si]
                cmp     bl, ' '
                jne     .End

                inc     cx
                inc     si
                jmp    .RightPad

.Error:         stc                     ; set CF
.End:           pop     dx              ; restore registers
                pop     bx
                ret
