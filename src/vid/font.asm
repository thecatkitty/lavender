%macro          DCHR    3
                                        ; VID_CHARACTER_DESCRIPTOR
                dw      %1              ;   CodePoint
                dw      %3              ;   Overlay
                db      %2              ;   Base

%endmacro


section .rodata.font


                                global  __VidFontData
__VidFontData:
                                DCHR    00A7h, 15h, 0   ; SECTION SIGN
                                DCHR    00B6h, 14h, 0   ; PILCROW SIGN
                                DCHR    00D3h, 'O', Acute
                                DCHR    00F3h, 'o', Acute
                                DCHR    0104h, 'A', Ogonek
                                DCHR    0105h, 'a', Ogonek
                                DCHR    0106h, 'C', Acute
                                DCHR    0107h, 'c', Acute
                                DCHR    0118h, 'E', Ogonek
                                DCHR    0119h, 'e', Ogonek
                                DCHR    0141h, 'L', Stroke
                                DCHR    0142h, 'l', Stroke
                                DCHR    0143h, 'N', Acute
                                DCHR    0144h, 'n', Acute
                                DCHR    015Ah, 'S', Acute
                                DCHR    015Bh, 's', Acute
                                DCHR    0179h, 'Z', Acute
                                DCHR    017Ah, 'z', Acute
                                DCHR    017Bh, 'Z', DotAbove
                                DCHR    017Ch, 'z', DotAbove
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
FONT_DATA_LENGTH                equ     ($ - __VidFontData) / 5
                                dw      0FFFFh

Acute:                          db      02h, \
                                        00011000b, \
                                        00110000b
DotAbove:                       db      01h, \
                                        00110000b
Ogonek:                         db      71h, \
                                        00011100b
Stroke:                         db      22h, \
                                        00011000b, \
                                        01100000b


section .bss


                                global  __VidExtendedFont
__VidExtendedFont               resb    FONT_DATA_LENGTH * 8
