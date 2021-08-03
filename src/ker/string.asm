%define KER_API_SUBSET_STRING
%include "ker.inc"


                cpu     8086

[bits 16]
section .text


                global  KerParseU16
KerParseU16:
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


                global  KerByteToHex
KerByteToHex:
                push    bx
                xor     bx, bx
                mov     bl, al
                and     bl, 0Fh

                and     ax, 00F0h
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                or      ax, bx
                add     ax, ('0' << 8) | '0'

                cmp     al, '9'
                jbe     .CheckHigh
                add     al, 'A' - '9' - 1
.CheckHigh:
                cmp     ah, '9'
                jbe     .End
                add     ah, 'A' - '9' - 1
.End:
                pop     bx
                ret


                global  KerCompareMemory
KerCompareMemory:
                push    si
                push    cx
                push    bx
                push    ax
.NextByte:
                mov     al, byte [si]
                cmp     al, byte [bx]
                jne     .End
                inc     si
                inc     bx
                loop    .NextByte
                cmp     ax, ax
.End:
                pop     ax
                pop     bx
                pop     cx
                pop     si
                ret


                global  KerIsWhitespace
KerIsWhitespace:
                cmp     al, 20h         ; SPACE
                jae     .End
                cmp     al, 0Dh         ; CARRIAGE RETURN
                jae     .End
                cmp     al, 0Ch         ; FORM FEED
                jae     .End
                cmp     al, 0Bh         ; VERTICAL TAB
                jae     .End
                cmp     al, 0Ah         ; LINE FEED
                jae     .End
                cmp     al, 09h         ; HORIZONTAL TAB
.End:           ret
