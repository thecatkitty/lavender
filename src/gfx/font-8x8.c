#include "glyph.h"

#define OVERLAY(name, top, height, ...)                                        \
    static const uint8_t name[height + 1] = {                                  \
        (((top) << 4) | ((height) & 0xF)), __VA_ARGS__}

#define TRANSFORMATION(name, ...)                                              \
    static const uint8_t name[] = {__VA_ARGS__, GXF_END}

// Character overlays
OVERLAY(_acute, 0, 1, 0x1C);
OVERLAY(_dot_above, 0, 1, 0x30);
OVERLAY(_ogonek, 6, 2, 0x0E, 0x07);
OVERLAY(_stroke, 2, 3, 0x04, 0x78, 0x80);
OVERLAY(_tilde, 0, 1, 0x7C);
OVERLAY(_diaeresis, 0, 1, 0xCC);
OVERLAY(_caron, 0, 2, 0x6C, 0x38);
OVERLAY(_ring, 0, 2, 0x30, 0x30);
OVERLAY(_apostrophe, 0, 1, 0x03);

// Character transformations
TRANSFORMATION(_move_two_top, //
               GXF_SELECT(0),
               GXF_GROW(1),
               GXF_MOVE(2));
TRANSFORMATION(_squish_upper_a, //
               GXF_SELECT(0),
               GXF_GROW(4),
               GXF_MOVE(1),
               GXF_SELECT(1),
               GXF_GROW(1),
               GXF_MOVE(1));
TRANSFORMATION(_squish_upper_e, //
               GXF_SELECT(0),
               GXF_GROW(1),
               GXF_MOVE(1),
               GXF_SELECT(1),
               GXF_GROW(2),
               GXF_MOVE(1));
TRANSFORMATION(_squish_upper_n, //
               GXF_SELECT(1),
               GXF_GROW(3),
               GXF_MOVE(1),
               GXF_CLEAR(0));
TRANSFORMATION(_squish_upper_y, //
               GXF_SELECT(1),
               GXF_GROW(2),
               GXF_MOVE(1),
               GXF_CLEAR(0));

// Character descriptors
const vid_glyph __vid_font_8x8[] = {
    {0x00A7, 0, 0, 0x15}, // SECTION SIGN
    {0x00B6, 0, 0, 0x14}, // PILCROW SIGN
    {0x00C1, _acute, _squish_upper_a, 'A'},
    {0x00C9, _acute, _squish_upper_e, 'E'},
    {0x00CD, _acute, _move_two_top, 'I'},
    {0x00D1, _tilde, _squish_upper_n, 'N'},
    {0x00D3, _acute, _move_two_top, 'O'},
    {0x00DA, _acute, _move_two_top, 'U'},
    {0x00DC, _diaeresis, _move_two_top, 'U'},
    {0x00DD, _acute, _squish_upper_y, 'Y'},
    {0x00E1, _acute, 0, 'a'},
    {0x00E9, _acute, 0, 'e'},
    {0x00ED, _acute, 0, 'i'},
    {0x00F1, _tilde, 0, 'n'},
    {0x00F3, _acute, 0, 'o'},
    {0x00FA, _acute, 0, 'u'},
    {0x00FC, _diaeresis, 0, 'u'},
    {0x00FD, _acute, 0, 'y'},
    {0x0104, _ogonek, 0, 'A'},
    {0x0105, _ogonek, 0, 'a'},
    {0x0106, _acute, _move_two_top, 'C'},
    {0x0107, _acute, 0, 'c'},
    {0x010C, _caron, _move_two_top, 'C'},
    {0x010D, _caron, 0, 'c'},
    {0x010E, _caron, _move_two_top, 'D'},
    {0x010F, _apostrophe, 0, 'd'},
    {0x0118, _ogonek, 0, 'E'},
    {0x0119, _ogonek, 0, 'e'},
    {0x011A, _caron, _squish_upper_e, 'E'},
    {0x011B, _caron, 0, 'e'},
    {0x0141, _stroke, 0, 'L'},
    {0x0142, _stroke, 0, 'l'},
    {0x0143, _acute, _squish_upper_n, 'N'},
    {0x0144, _acute, 0, 'n'},
    {0x0147, _caron, _squish_upper_n, 'N'},
    {0x0148, _caron, 0, 'n'},
    {0x0158, _caron, _move_two_top, 'R'},
    {0x0159, _caron, 0, 'r'},
    {0x015A, _acute, 0, 's'},
    {0x015B, _acute, 0, 's'},
    {0x0160, _caron, 0, 's'},
    {0x0161, _caron, 0, 's'},
    {0x0164, _caron, _move_two_top, 'T'},
    {0x0165, _apostrophe, 0, 't'},
    {0x016E, _ring, _move_two_top, 'U'},
    {0x016F, _ring, 0, 'u'},
    {0x0179, _acute, 0, 'z'},
    {0x017A, _acute, 0, 'z'},
    {0x017B, _dot_above, 0, 'z'},
    {0x017C, _dot_above, 0, 'z'},
    {0x017D, _caron, 0, 'z'},
    {0x017E, _caron, 0, 'z'},
    {0x2022, 0, 0, 0x07}, // BULLET
    {0x203C, 0, 0, 0x13}, // DOUBLE EXCLAMATION MARK
    {0x2190, 0, 0, 0x1B}, // LEFTWARDS ARROW
    {0x2191, 0, 0, 0x18}, // UPWARDS ARROW
    {0x2192, 0, 0, 0x1A}, // RIGHTWARDS ARROW
    {0x2193, 0, 0, 0x19}, // DOWNWARDS ARROW
    {0x2194, 0, 0, 0x1D}, // LEFT RIGHT ARROW
    {0x2195, 0, 0, 0x12}, // UP DOWN ARROW
    {0x21A8, 0, 0, 0x17}, // UP DOWN ARROW WITH BASE
    {0x221F, 0, 0, 0x1C}, // RIGHT ANGLE
    {0x2302, 0, 0, 0x7F}, // HOUSE
    {0x25AC, 0, 0, 0x16}, // BLACK RECTANGLE
    {0x25B2, 0, 0, 0x1E}, // BLACK UP-POINTING TRIANGLE
    {0x25BA, 0, 0, 0x10}, // BLACK RIGHT-POINTING POINTER
    {0x25BC, 0, 0, 0x1F}, // BLACK DOWN-POINTING TRIANGLE
    {0x25C4, 0, 0, 0x11}, // BLACK LEFT-POINTING POINTER
    {0x25CB, 0, 0, 0x09}, // WHITE CIRCLE
    {0x25D8, 0, 0, 0x08}, // INVERSE BULLET
    {0x25D9, 0, 0, 0x0A}, // INVERSE WHITE CIRCLE
    {0x263A, 0, 0, 0x01}, // WHITE SMILING FACE
    {0x263B, 0, 0, 0x02}, // BLACK SMILING FACE
    {0x263C, 0, 0, 0x0F}, // WHITE SUN WITH RAYS
    {0x2640, 0, 0, 0x0C}, // FEMALE SIGN
    {0x2642, 0, 0, 0x0B}, // MALE SIGN
    {0x2660, 0, 0, 0x06}, // BLACK SPADE SUIT
    {0x2663, 0, 0, 0x05}, // BLACK CLUB SUIT
    {0x2665, 0, 0, 0x03}, // BLACK HEART SUIT
    {0x2666, 0, 0, 0x04}, // BLACK DIAMOND SUIT
    {0x266A, 0, 0, 0x0D}, // EIGHTH NOTE
    {0x266B, 0, 0, 0x0E}, // BEAMED EIGHTH NOTES
    {0xFFFF}};

char __vid_xfont[8 * sizeof(__vid_font_8x8) / sizeof(vid_glyph)];
