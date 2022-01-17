.macro          DCHR    CodePoint, Base, Overlay

                .word   \CodePoint
                .word   \Overlay
                .byte   \Base

.endm


.section .rodata.font


                                .global __VidFontData
__VidFontData:
                                DCHR    0x00A7, 0x15, 0   // SECTION SIGN
                                DCHR    0x00B6, 0x14, 0   // PILCROW SIGN
                                DCHR    0x00D3, 'O', Acute
                                DCHR    0x00F3, 'o', Acute
                                DCHR    0x0104, 'A', Ogonek
                                DCHR    0x0105, 'a', Ogonek
                                DCHR    0x0106, 'C', Acute
                                DCHR    0x0107, 'c', Acute
                                DCHR    0x0118, 'E', Ogonek
                                DCHR    0x0119, 'e', Ogonek
                                DCHR    0x0141, 'L', Stroke
                                DCHR    0x0142, 'l', Stroke
                                DCHR    0x0143, 'N', Acute
                                DCHR    0x0144, 'n', Acute
                                DCHR    0x015A, 'S', Acute
                                DCHR    0x015B, 's', Acute
                                DCHR    0x0179, 'Z', Acute
                                DCHR    0x017A, 'z', Acute
                                DCHR    0x017B, 'Z', DotAbove
                                DCHR    0x017C, 'z', DotAbove
                                DCHR    0x2022, 0x07, 0   // BULLET
                                DCHR    0x203C, 0x13, 0   // DOUBLE EXCLAMATION MARK
                                DCHR    0x2190, 0x1B, 0   // LEFTWARDS ARROW
                                DCHR    0x2191, 0x18, 0   // UPWARDS ARROW
                                DCHR    0x2192, 0x1A, 0   // RIGHTWARDS ARROW
                                DCHR    0x2193, 0x19, 0   // DOWNWARDS ARROW
                                DCHR    0x2194, 0x1D, 0   // LEFT RIGHT ARROW
                                DCHR    0x2195, 0x12, 0   // UP DOWN ARROW
                                DCHR    0x21A8, 0x17, 0   // UP DOWN ARROW WITH BASE
                                DCHR    0x221F, 0x1C, 0   // RIGHT ANGLE
                                DCHR    0x2302, 0x7F, 0   // HOUSE
                                DCHR    0x25AC, 0x16, 0   // BLACK RECTANGLE
                                DCHR    0x25B2, 0x1E, 0   // BLACK UP-POINTING TRIANGLE
                                DCHR    0x25BA, 0x10, 0   // BLACK RIGHT-POINTING POINTER
                                DCHR    0x25BC, 0x1F, 0   // BLACK DOWN-POINTING TRIANGLE
                                DCHR    0x25C4, 0x11, 0   // BLACK LEFT-POINTING POINTER
                                DCHR    0x25CB, 0x09, 0   // WHITE CIRCLE
                                DCHR    0x25D8, 0x08, 0   // INVERSE BULLET
                                DCHR    0x25D9, 0x0A, 0   // INVERSE WHITE CIRCLE
                                DCHR    0x263A, 0x01, 0   // WHITE SMILING FACE
                                DCHR    0x263B, 0x02, 0   // BLACK SMILING FACE
                                DCHR    0x263C, 0x0F, 0   // WHITE SUN WITH RAYS
                                DCHR    0x2640, 0x0C, 0   // FEMALE SIGN
                                DCHR    0x2642, 0x0B, 0   // MALE SIGN
                                DCHR    0x2660, 0x06, 0   // BLACK SPADE SUIT
                                DCHR    0x2663, 0x05, 0   // BLACK CLUB SUIT
                                DCHR    0x2665, 0x03, 0   // BLACK HEART SUIT
                                DCHR    0x2666, 0x04, 0   // BLACK DIAMOND SUIT
                                DCHR    0x266A, 0x0D, 0   // EIGHTH NOTE
                                DCHR    0x266B, 0x0E, 0   // BEAMED EIGHTH NOTES
__VidFontDataEnd:               .word   0x0FFFF

Acute:                          .byte   0x02, \
                                        0b00011000, \
                                        0b00110000
DotAbove:                       .byte   0x01, \
                                        0b00110000
Ogonek:                         .byte   0x71, \
                                        0b00011100
Stroke:                         .byte   0x22, \
                                        0b00011000, \
                                        0b01100000


                                .global __VidExtendedFont
.lcomm __VidExtendedFont,       (__VidFontDataEnd - __VidFontData) / 5 * 8
