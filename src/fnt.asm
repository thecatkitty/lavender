%define FNT_API
%include "dos.inc"
%include "fnt.inc"


                cpu     8086

[bits 16]
section .text

                global  FntLoad
FntLoad:
                push    es              ; preserve registers
                push    bp
                push    di
                push    si
                push    dx
                push    cx
                push    bx
                push    ax

                xor     ax, ax
                mov     es, ax          ; preserve previous extended font pointer
                push    word [es:FNT_INTV_EXTENDED_FONT_PTR * 4]
                push    word [es:FNT_INTV_EXTENDED_FONT_PTR * 4 + 2]
                pop     word [lpOldPointer + 2]
                pop     word [lpOldPointer]
                mov     word [es:FNT_INTV_EXTENDED_FONT_PTR * 4], bmExtendedFont
                mov     word [es:FNT_INTV_EXTENDED_FONT_PTR * 4 + 2], ds

                mov     ax, FNT_BASIC_FONT_SEGMENT
                mov     es, ax
                mov     si, aFontData
                mov     di, bmExtendedFont
                inc     di
                mov     dx, lFontData
                cmp     dx, 0
.NextCharacter:
                jz      .End
                xor     ax, ax
                mov     al, byte [si + 4]
                cmp     al, 0
                jz      .ApplyOverlay
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                mov     bx, FNT_BASIC_FONT_OFFSET
                add     bx, ax
                mov     cx, FNT_CHARACTER_HEIGHT / 2
                push    di              ; preserve extended glyph position
.NextBasicWord:
                mov     ax, word [es:bx]
                mov     word [di], ax
                add     bx, 2
                add     di, 2
                loop    .NextBasicWord
.ApplyOverlay:
                mov     bx, word [si + 2]
                xor     ax, ax
                xor     cx, cx
                mov     al, byte [bx]   ; 7:4 - position from top, 3:0 - height
                mov     cl, al
                shr     al, 1
                shr     al, 1
                shr     al, 1
                shr     al, 1           ; AX = position from top
                and     cl, 0Fh         ; CX = overlay height
                jcxz    .OverlayEnd

                mov     bp, sp
                mov     di, word [bp]   ; restore extended glyph position
                add     di, ax          ; start from top of the overlay
                inc     bx
.NextOverlayByte:
                mov     al, byte [bx]   ; read a byte from the overlay
                or      byte [di], al   ; apply the overlay by ORing
                inc     bx
                inc     di
                loop    .NextOverlayByte
.OverlayEnd:
                pop     di              ; restore extended glyph position
                add     di, FNT_CHARACTER_HEIGHT
                add     si, 5
                dec     dx
                jmp     .NextCharacter
.End:
                pop     ax
                pop     bx
                pop     cx
                pop     dx
                pop     si
                pop     di
                pop     bp
                pop     es
                ret


                global  FntUnload
FntUnload:
                push    es              ; preserve registers
                push    ax

                xor     ax, ax
                mov     es, ax          ; restore previous extended font pointer
                push    word [lpOldPointer]
                push    word [lpOldPointer + 2]
                pop     word [es:FNT_INTV_EXTENDED_FONT_PTR * 4 + 2]
                pop     word [es:FNT_INTV_EXTENDED_FONT_PTR * 4]
                
                pop     ax
                pop     es
                ret


                global  FntGetLocalCharacter
FntGetLocalCharacter:
                cmp     ax, 80h
                jae     .Convert
                ret                     ; do not convert ASCII code points
.Convert:
                push    si
                push    cx
                mov     si, aFontData
                xor     cx, cx
.NextCharacter:
                cmp     cx, lFontData
                je      .Error
                cmp     word [si], ax
                je      .Return
                add     si, 5
                inc     cx
                jmp     .NextCharacter
.Error:
                mov     al, '?'
                stc
                jmp     .End
.Return:
                mov     al, cl
                add     al, 80h
.End:
                pop     cx
                pop     si
                ret


section .data
%push Context

%macro          DCHR    3

                dw      %1, %3
                db      %2

%endmacro


aFontData:                      DCHR    0104h, 'A', bmOgonek
                                DCHR    0106h, 'C', bmAcute
                                DCHR    0118h, 'E', bmOgonek
                                DCHR    0141h, 'L', bmStroke
                                DCHR    0143h, 'N', bmAcute
                                DCHR    00D3h, 'O', bmAcute
                                DCHR    015Ah, 'S', bmAcute
                                DCHR    0179h, 'Z', bmAcute
                                DCHR    017Bh, 'Z', bmDotAbove
                                DCHR    0105h, 'a', bmOgonek
                                DCHR    0107h, 'c', bmAcute
                                DCHR    0119h, 'e', bmOgonek
                                DCHR    0142h, 'l', bmStroke
                                DCHR    0144h, 'n', bmAcute
                                DCHR    00F3h, 'o', bmAcute
                                DCHR    015Bh, 's', bmAcute
                                DCHR    017Ah, 'z', bmAcute
                                DCHR    017Ch, 'z', bmDotAbove
lFontData                       equ     ($ - aFontData) / 5

bmAcute:                        db      02h, \
                                        00011000b, \
                                        00110000b
bmDotAbove:                     db      01h, \
                                        00110000b
bmOgonek:                       db      71h, \
                                        00011100b
bmStroke:                       db      22h, \
                                        00011000b, \
                                        01100000b

%pop


section .bss


bmExtendedFont                  resb    lFontData * 8
lpOldPointer                    resd    1
