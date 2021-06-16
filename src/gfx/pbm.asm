%define PIC_API
%include "err.inc"
%include "pic.inc"
%include "str.inc"


                cpu     8086

[bits 16]
section .text


                global  GfxLoadBitmap
GfxLoadBitmap:
                cmp     word [si], PIC_PBM_RAW_MAGIC
                je      PbmLoadBitmap
                ERR     PIC_UNSUPPORTED_FORMAT
.Error:         ret


PbmLoadBitmap:
                push    si
                push    ax
                push    cx

                add     si, 2
.LookForWidth:
                mov     al, byte [si]
                call    KerIsWhitespace
                jne     .ReadWidth
                inc     si
                jmp     .LookForWidth
.ReadWidth:
                call    PbmParseU16
                jc      .Error
                mov     word [di + PIC_BITMAP.wWidth], ax
                test    ax, 00000000_00000111b
                jz      .DoNotAdd
                add     ax, 8
.DoNotAdd:      mov     cl, 3
                shr     ax, cl
                mov     word [di + PIC_BITMAP.wWidthBytes], ax
.LookForHeight:
                mov     al, byte [si]
                call    KerIsWhitespace
                jne     .ReadHeight
                inc     si
                jmp     .LookForHeight
.ReadHeight:
                call    PbmParseU16
                jc      .Error
                mov     word [di + PIC_BITMAP.wHeight], ax
.LookForBits:
                mov     al, byte [si]
                call    KerIsWhitespace
                jne     .SaveBits
                inc     si
                jmp     .LookForBits
.SaveBits:
                mov     byte [di + PIC_BITMAP.bPlanes], 1
                mov     byte [di + PIC_BITMAP.bBitsPerPixel], 1
                mov     word [di + PIC_BITMAP.pBits], si
                clc
.Error:
.End:           pop     cx
                pop     ax
                pop     si
                ret


; Parse an unsigned 16-bit integer in the PBM header
; Input:
;   DS:SI - string
; Output:
;   AX    - parsed integer
;   CF    - parsing error
;   DS:SI - first cell after the field
PbmParseU16:
                push    bx
                push    cx
                xor     ah, ah
                xor     cx, cx
.NextCharacter:
                mov     al, byte [si]
                cmp     al, '#'
                je      .SkipComment
                call    KerIsWhitespace
                je      .End
                cmp     al, '0'
                jb      .Bad
                cmp     al, '9'
                ja      .Bad
                sub     al, '0'
                mov     bx, cx
                shl     bx, 1
                shl     bx, 1                           ; BX = CX * 4
                add     bx, cx
                shl     bx, 1                           ; BX = CX * 10
                add     bx, ax
                mov     cx, bx
                inc     si
                jmp     .NextCharacter
.SkipComment:
                inc     si
                cmp     byte [si - 1], 10               ; LINE FEED
                je      .NextCharacter
                jmp     .SkipComment
.Bad:
                ERR     PIC_MALFORMED_FILE
.Error:
.End:           mov     ax, cx
                pop     cx
                pop     bx
                ret
