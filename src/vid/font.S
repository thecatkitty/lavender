#include <vid.h>

.macro          DCHR    CodePoint, Base, Overlay, Transformation

                .word   \CodePoint
                .word   \Overlay
                .word   \Transformation
                .byte   \Base

.endm

.section .rodata.font


                                .global __VidFontData
__VidFontData:
                                DCHR    0x00A7, 0x15, 0, 0 // SECTION SIGN
                                DCHR    0x00B6, 0x14, 0, 0 // PILCROW SIGN
                                DCHR    0x00C1, 'A', Acute, SquishCapitalA
                                DCHR    0x00C9, 'E', Acute, SquishCapitalE
                                DCHR    0x00CD, 'I', Acute, MoveTwoUpper
                                DCHR    0x00D1, 'N', Tilde, SquishCapitalN
                                DCHR    0x00D3, 'O', Acute, MoveTwoUpper
                                DCHR    0x00DA, 'U', Acute, MoveTwoUpper
                                DCHR    0x00DC, 'U', Diaeresis, MoveTwoUpper
                                DCHR    0x00DD, 'Y', Acute, SquishCapitalY
                                DCHR    0x00E1, 'a', Acute, 0
                                DCHR    0x00E9, 'e', Acute, 0
                                DCHR    0x00ED, 'i', Acute, 0
                                DCHR    0x00F1, 'n', Tilde, 0
                                DCHR    0x00F3, 'o', Acute, 0
                                DCHR    0x00FA, 'u', Acute, 0
                                DCHR    0x00FC, 'u', Diaeresis, 0
                                DCHR    0x00FD, 'y', Acute, 0
                                DCHR    0x0104, 'A', Ogonek, 0
                                DCHR    0x0105, 'a', Ogonek, 0
                                DCHR    0x0106, 'C', Acute, MoveTwoUpper
                                DCHR    0x0107, 'c', Acute, 0
                                DCHR    0x0118, 'E', Ogonek, 0
                                DCHR    0x0119, 'e', Ogonek, 0
                                DCHR    0x0141, 'L', Stroke, 0
                                DCHR    0x0142, 'l', Stroke, 0
                                DCHR    0x0143, 'N', Acute, SquishCapitalN
                                DCHR    0x0144, 'n', Acute, 0
                                DCHR    0x015A, 's', Acute, 0
                                DCHR    0x015B, 's', Acute, 0
                                DCHR    0x0179, 'z', Acute, 0
                                DCHR    0x017A, 'z', Acute, 0
                                DCHR    0x017B, 'z', DotAbove, 0
                                DCHR    0x017C, 'z', DotAbove, 0
                                DCHR    0x2022, 0x07, 0, 0 // BULLET
                                DCHR    0x203C, 0x13, 0, 0 // DOUBLE EXCLAMATION MARK
                                DCHR    0x2190, 0x1B, 0, 0 // LEFTWARDS ARROW
                                DCHR    0x2191, 0x18, 0, 0 // UPWARDS ARROW
                                DCHR    0x2192, 0x1A, 0, 0 // RIGHTWARDS ARROW
                                DCHR    0x2193, 0x19, 0, 0 // DOWNWARDS ARROW
                                DCHR    0x2194, 0x1D, 0, 0 // LEFT RIGHT ARROW
                                DCHR    0x2195, 0x12, 0, 0 // UP DOWN ARROW
                                DCHR    0x21A8, 0x17, 0, 0 // UP DOWN ARROW WITH BASE
                                DCHR    0x221F, 0x1C, 0, 0 // RIGHT ANGLE
                                DCHR    0x2302, 0x7F, 0, 0 // HOUSE
                                DCHR    0x25AC, 0x16, 0, 0 // BLACK RECTANGLE
                                DCHR    0x25B2, 0x1E, 0, 0 // BLACK UP-POINTING TRIANGLE
                                DCHR    0x25BA, 0x10, 0, 0 // BLACK RIGHT-POINTING POINTER
                                DCHR    0x25BC, 0x1F, 0, 0 // BLACK DOWN-POINTING TRIANGLE
                                DCHR    0x25C4, 0x11, 0, 0 // BLACK LEFT-POINTING POINTER
                                DCHR    0x25CB, 0x09, 0, 0 // WHITE CIRCLE
                                DCHR    0x25D8, 0x08, 0, 0 // INVERSE BULLET
                                DCHR    0x25D9, 0x0A, 0, 0 // INVERSE WHITE CIRCLE
                                DCHR    0x263A, 0x01, 0, 0 // WHITE SMILING FACE
                                DCHR    0x263B, 0x02, 0, 0 // BLACK SMILING FACE
                                DCHR    0x263C, 0x0F, 0, 0 // WHITE SUN WITH RAYS
                                DCHR    0x2640, 0x0C, 0, 0 // FEMALE SIGN
                                DCHR    0x2642, 0x0B, 0, 0 // MALE SIGN
                                DCHR    0x2660, 0x06, 0, 0 // BLACK SPADE SUIT
                                DCHR    0x2663, 0x05, 0, 0 // BLACK CLUB SUIT
                                DCHR    0x2665, 0x03, 0, 0 // BLACK HEART SUIT
                                DCHR    0x2666, 0x04, 0, 0 // BLACK DIAMOND SUIT
                                DCHR    0x266A, 0x0D, 0, 0 // EIGHTH NOTE
                                DCHR    0x266B, 0x0E, 0, 0 // BEAMED EIGHTH NOTES
__VidFontDataEnd:               .word   0x0FFFF

Acute:                          .byte   0x01, \
                                        0b00011100
DotAbove:                       .byte   0x01, \
                                        0b00110000
Ogonek:                         .byte   0x62, \
                                        0b00001110, \
                                        0b00000111
Stroke:                         .byte   0x23, \
                                        0b00000100, \
                                        0b01111000, \
                                        0b10000000
Tilde:                          .byte   0x01, \
                                        0b01111100
Diaeresis:                      .byte   0x01, \
                                        0b11001100

MoveTwoUpper:                   .byte   GXF_SELECT(0), \
                                        GXF_GROW(1), \
                                        GXF_MOVE(2), \
                                        GXF_END
SquishCapitalA:                 .byte   GXF_SELECT(0), \
                                        GXF_GROW(4), \
                                        GXF_MOVE(1), \
                                        GXF_SELECT(1), \
                                        GXF_GROW(1), \
                                        GXF_MOVE(1), \
                                        GXF_END
SquishCapitalE:                 .byte   GXF_SELECT(0), \
                                        GXF_GROW(1), \
                                        GXF_MOVE(1), \
                                        GXF_SELECT(1), \
                                        GXF_GROW(2), \
                                        GXF_MOVE(1), \
                                        GXF_END
SquishCapitalN:                 .byte   GXF_SELECT(1), \
                                        GXF_GROW(3), \
                                        GXF_MOVE(1), \
                                        GXF_CLEAR(0), \
                                        GXF_END
SquishCapitalY:                 .byte   GXF_SELECT(1), \
                                        GXF_GROW(2), \
                                        GXF_MOVE(1), \
                                        GXF_CLEAR(0), \
                                        GXF_END

                                .global __VidExtendedFont
.lcomm __VidExtendedFont,       (__VidFontDataEnd - __VidFontData) / 7 * 8
