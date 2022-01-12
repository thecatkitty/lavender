%define VID_API
%include "ker.inc"
%include "vid.inc"
%include "dev/cga.inc"


section .data
%push Context

%macro          DCHR    3
                                        ; VID_CHARACTER_DESCRIPTOR
                dw      %1              ;   .wcCodePoint
                dw      %3              ;   .pabOverlay
                db      %2              ;   .cBase

%endmacro


                                global  astFontData
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


                                global  abExtendedFont
abExtendedFont                  resb    nFontDataLength * 8

                                global  lpabPreviousFont
lpabPreviousFont                resd    1
