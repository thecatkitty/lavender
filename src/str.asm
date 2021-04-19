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

.lpad:
                mov     bl, [si]
                cmp     bl, ' '
                jne     .digit

                inc     cx
                inc     si
                jmp    .lpad

.digit:
                mov     bl, [si]
                cmp     bl, '0'
                jb      .below_digit
                cmp     bl, '9'
                ja      .error          ; invalid character

                mov     dx, 10          ; multiplier
                mul     dx
                sub     bl, '0'
                add     ax, bx          ; AX = (AX * 10) mod 65536 + BL

                inc     cx
                inc     si
                jmp    .digit

.below_digit:
                cmp     bl, ' '
                jne     .error          ; invalid character

                inc     cx
                inc     si

.rpad:
                mov     bl, [si]
                cmp     bl, ' '
                jne     .end

                inc     cx
                inc     si
                jmp    .rpad

.error:         stc                     ; set CF
.end:           pop     dx              ; restore registers
                pop     bx
                ret
