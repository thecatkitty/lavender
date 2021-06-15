%define VID_API
%include "ker.inc"
%include "vid.inc"


                cpu     8086

[bits 16]
section .text

                global  VidLoadFont
VidLoadFont:
                push    es              ; preserve registers
                push    bp
                push    di
                push    si
                push    cx
                push    bx
                push    ax

                xor     ax, ax
                mov     es, ax          ; preserve previous extended font pointer
                push    word [es:VID_INT_EXTENDED_FONT_PTR * 4]
                push    word [es:VID_INT_EXTENDED_FONT_PTR * 4 + 2]
                pop     word [lpabPreviousFont + 2]
                pop     word [lpabPreviousFont]
                mov     word [es:VID_INT_EXTENDED_FONT_PTR * 4], abExtendedFont
                mov     word [es:VID_INT_EXTENDED_FONT_PTR * 4 + 2], ds

                mov     ax, VID_BASIC_FONT_SEGMENT
                mov     es, ax
                mov     si, astFontData
                mov     di, abExtendedFont

                call    KerIsDosBox
                jnz     .NextCharacter
                inc     di              ; DOSBox ROM font is moved one line lower
.NextCharacter:
                cmp     word [si + VID_CHARACTER_DESCRIPTOR.wcCodePoint], 0FFFFh
                je      .End
                xor     ax, ax
                mov     al, byte [si + VID_CHARACTER_DESCRIPTOR.cBase]
                cmp     al, 0
                jz      .ApplyOverlay
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                mov     bx, VID_BASIC_FONT_OFFSET
                add     bx, ax
                mov     cx, VID_CHARACTER_HEIGHT / 2
                push    di              ; preserve extended glyph position
.NextBasicWord:
                mov     ax, word [es:bx]
                mov     word [di], ax
                add     bx, 2
                add     di, 2
                loop    .NextBasicWord
.ApplyOverlay:
                mov     bx, word [si + VID_CHARACTER_DESCRIPTOR.pOverlay]
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
                add     di, VID_CHARACTER_HEIGHT
                add     si, 5
                jmp     .NextCharacter
.End:
                pop     ax
                pop     bx
                pop     cx
                pop     si
                pop     di
                pop     bp
                pop     es
                ret


                global  VidUnloadFont
VidUnloadFont:
                push    es              ; preserve registers
                push    ax

                xor     ax, ax
                mov     es, ax          ; restore previous extended font pointer
                push    word [lpabPreviousFont]
                push    word [lpabPreviousFont + 2]
                pop     word [es:VID_INT_EXTENDED_FONT_PTR * 4 + 2]
                pop     word [es:VID_INT_EXTENDED_FONT_PTR * 4]
                
                pop     ax
                pop     es
                ret


                global  VidGetFontEncoding
VidGetFontEncoding:
                cmp     ax, 80h
                jae     .Convert
                ret                     ; do not convert ASCII code points
.Convert:
                push    si
                push    cx
                mov     si, astFontData
                xor     cx, cx
.NextCharacter:
                cmp     word [si + VID_CHARACTER_DESCRIPTOR.wcCodePoint], ax
                ja      .Error
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
                                        ; VID_CHARACTER_DESCRIPTOR
                dw      %1              ;   CodePoint
                dw      %3              ;   Overlay
                db      %2              ;   Base

%endmacro


astFontData:
                                DCHR    00D3h, 'O', abAcute
                                DCHR    00F3h, 'o', abAcute
                                DCHR    0104h, 'A', abOgonek
                                DCHR    0105h, 'a', abOgonek
                                DCHR    0106h, 'C', abAcute
                                DCHR    0107h, 'c', abAcute
                                DCHR    0118h, 'E', abOgonek
                                DCHR    0119h, 'e', abOgonek
                                DCHR    0141h, 'L', abStroke
                                DCHR    0142h, 'l', abStroke
                                DCHR    0143h, 'N', abAcute
                                DCHR    0144h, 'n', abAcute
                                DCHR    015Ah, 'S', abAcute
                                DCHR    015Bh, 's', abAcute
                                DCHR    0179h, 'Z', abAcute
                                DCHR    017Ah, 'z', abAcute
                                DCHR    017Bh, 'Z', abDotAbove
                                DCHR    017Ch, 'z', abDotAbove
nFontDataLength                 equ     ($ - astFontData) / 5
                                dw      0FFFFh

abAcute:                        db      02h, \
                                        00011000b, \
                                        00110000b
abDotAbove:                     db      01h, \
                                        00110000b
abOgonek:                       db      71h, \
                                        00011100b
abStroke:                       db      22h, \
                                        00011000b, \
                                        01100000b

%pop


section .bss


abExtendedFont                  resb    nFontDataLength * 8
lpabPreviousFont                resd    1
