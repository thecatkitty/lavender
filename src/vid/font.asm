%define VID_API
%include "ker.inc"
%include "vid.inc"
%include "dev/cga.inc"


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
                
                mov     ax, INT_CGA_EXTENDED_FONT_PTR
                push    ax
                push    ds
                mov     ax, abExtendedFont
                push    ax
                call    KerInstallIsr
                add     sp, 6
                mov     word [lpabPreviousFont], ax
                mov     word [lpabPreviousFont + 2], dx

                mov     ax, CGA_BASIC_FONT_SEGMENT
                mov     es, ax
                mov     si, astFontData
                mov     di, abExtendedFont

                push    ax
                push    bx
                push    cx
                push    dx
                call    KerIsDosBox
                cmp     ax, 0
                pop     dx
                pop     cx
                pop     bx
                pop     ax
                je      .NextCharacter
                inc     di              ; DOSBox ROM font is moved one line lower
.NextCharacter:
                cmp     word [si + VID_CHARACTER_DESCRIPTOR.wcCodePoint], 0FFFFh
                je      .End
                cmp     word [si + VID_CHARACTER_DESCRIPTOR.pabOverlay], 0
                jnz     .Compose
                add     si, VID_CHARACTER_DESCRIPTOR_size
                jmp     .NextCharacter
.Compose:
                xor     ax, ax
                mov     al, byte [si + VID_CHARACTER_DESCRIPTOR.cBase]
                cmp     al, 0
                jz      .ApplyOverlay
                shl     ax, 1
                shl     ax, 1
                shl     ax, 1
                mov     bx, CGA_BASIC_FONT_OFFSET
                add     bx, ax
                mov     cx, CGA_CHARACTER_HEIGHT / 2
                push    di              ; preserve extended glyph position
.NextBasicWord:
                mov     ax, word [es:bx]
                mov     word [di], ax
                add     bx, 2
                add     di, 2
                loop    .NextBasicWord
.ApplyOverlay:
                mov     bx, word [si + VID_CHARACTER_DESCRIPTOR.pabOverlay]
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
                add     di, CGA_CHARACTER_HEIGHT
                add     si, VID_CHARACTER_DESCRIPTOR_size
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
                
                mov     ax, INT_CGA_EXTENDED_FONT_PTR
                push    ax
                mov     ax, word [lpabPreviousFont + 2]
                push    ax
                mov     ax, word [lpabPreviousFont]
                push    ax
                call    KerUninstallIsr
                add     sp, 6
                
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
                cmp     word [si + VID_CHARACTER_DESCRIPTOR.pabOverlay], 0
                je      .JumpNext
                inc     cx
.JumpNext:
                add     si, VID_CHARACTER_DESCRIPTOR_size
                jmp     .NextCharacter
.Error:
                mov     al, '?'
                stc
                jmp     .End
.Return:
                cmp     word [si + VID_CHARACTER_DESCRIPTOR.pabOverlay], 0
                je      .Basic
                mov     al, cl
                add     al, 80h
                jmp     .End
.Basic:
                mov     al, byte [si + VID_CHARACTER_DESCRIPTOR.cBase]
.End:
                pop     cx
                pop     si
                ret


section .data
%push Context

%macro          DCHR    3
                                        ; VID_CHARACTER_DESCRIPTOR
                dw      %1              ;   .wcCodePoint
                dw      %3              ;   .pabOverlay
                db      %2              ;   .cBase

%endmacro


astFontData:
                                DCHR    00A7h, 15h, 0   ; SECTION SIGN
                                DCHR    00B6h, 14h, 0   ; PILCROW SIGN
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
                                DCHR    2022h, 07h, 0   ; BULLET
                                DCHR    203Ch, 13h, 0   ; DOUBLE EXCLAMATION MARK
                                DCHR    2190h, 1Bh, 0   ; LEFTWARDS ARROW
                                DCHR    2191h, 18h, 0   ; UPWARDS ARROW
                                DCHR    2192h, 1Ah, 0   ; RIGHTWARDS ARROW
                                DCHR    2193h, 19h, 0   ; DOWNWARDS ARROW
                                DCHR    2194h, 1Dh, 0   ; LEFT RIGHT ARROW
                                DCHR    2195h, 12h, 0   ; UP DOWN ARROW
                                DCHR    21A8h, 17h, 0   ; UP DOWN ARROW WITH BASE
                                DCHR    221Fh, 1Ch, 0   ; RIGHT ANGLE
                                DCHR    2302h, 7Fh, 0   ; HOUSE
                                DCHR    25ACh, 16h, 0   ; BLACK RECTANGLE
                                DCHR    25B2h, 1Eh, 0   ; BLACK UP-POINTING TRIANGLE
                                DCHR    25BAh, 10h, 0   ; BLACK RIGHT-POINTING POINTER
                                DCHR    25BCh, 1Fh, 0   ; BLACK DOWN-POINTING TRIANGLE
                                DCHR    25C4h, 11h, 0   ; BLACK LEFT-POINTING POINTER
                                DCHR    25CBh, 09h, 0   ; WHITE CIRCLE
                                DCHR    25D8h, 08h, 0   ; INVERSE BULLET
                                DCHR    25D9h, 0Ah, 0   ; INVERSE WHITE CIRCLE
                                DCHR    263Ah, 01h, 0   ; WHITE SMILING FACE
                                DCHR    263Bh, 02h, 0   ; BLACK SMILING FACE
                                DCHR    263Ch, 0Fh, 0   ; WHITE SUN WITH RAYS
                                DCHR    2640h, 0Ch, 0   ; FEMALE SIGN
                                DCHR    2642h, 0Bh, 0   ; MALE SIGN
                                DCHR    2660h, 06h, 0   ; BLACK SPADE SUIT
                                DCHR    2663h, 05h, 0   ; BLACK CLUB SUIT
                                DCHR    2665h, 03h, 0   ; BLACK HEART SUIT
                                DCHR    2666h, 04h, 0   ; BLACK DIAMOND SUIT
                                DCHR    266Ah, 0Dh, 0   ; EIGHTH NOTE
                                DCHR    266Bh, 0Eh, 0   ; BEAMED EIGHTH NOTES
nFontDataLength                 equ     ($ - astFontData) / VID_CHARACTER_DESCRIPTOR_size
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
