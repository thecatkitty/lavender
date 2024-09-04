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
const gfx_glyph __vid_font_8x8[] = {
    {0x00A7, 0x15}, // SECTION SIGN
    {0x00B6, 0x14}, // PILCROW SIGN
    {0x00C1, 'A', _acute, _squish_upper_a},
    {0x00C9, 'E', _acute, _squish_upper_e},
    {0x00CD, 'I', _acute, _move_two_top},
    {0x00D1, 'N', _tilde, _squish_upper_n},
    {0x00D3, 'O', _acute, _move_two_top},
    {0x00DA, 'U', _acute, _move_two_top},
    {0x00DC, 'U', _diaeresis, _move_two_top},
    {0x00DD, 'Y', _acute, _squish_upper_y},
    {0x00E1, 'a', _acute},
    {0x00E9, 'e', _acute},
    {0x00ED, 'i', _acute},
    {0x00F1, 'n', _tilde},
    {0x00F3, 'o', _acute},
    {0x00FA, 'u', _acute},
    {0x00FC, 'u', _diaeresis},
    {0x00FD, 'y', _acute},
    {0x0104, 'A', _ogonek},
    {0x0105, 'a', _ogonek},
    {0x0106, 'C', _acute, _move_two_top},
    {0x0107, 'c', _acute},
    {0x010C, 'C', _caron, _move_two_top},
    {0x010D, 'c', _caron},
    {0x010E, 'D', _caron, _move_two_top},
    {0x010F, 'd', _apostrophe},
    {0x0118, 'E', _ogonek},
    {0x0119, 'e', _ogonek},
    {0x011A, 'E', _caron, _squish_upper_e},
    {0x011B, 'e', _caron},
    {0x0141, 'L', _stroke},
    {0x0142, 'l', _stroke},
    {0x0143, 'N', _acute, _squish_upper_n},
    {0x0144, 'n', _acute},
    {0x0147, 'N', _caron, _squish_upper_n},
    {0x0148, 'n', _caron},
    {0x0158, 'R', _caron, _move_two_top},
    {0x0159, 'r', _caron},
    {0x015A, 's', _acute},
    {0x015B, 's', _acute},
    {0x0160, 's', _caron},
    {0x0161, 's', _caron},
    {0x0164, 'T', _caron, _move_two_top},
    {0x0165, 't', _apostrophe},
    {0x016E, 'U', _ring, _move_two_top},
    {0x016F, 'u', _ring},
    {0x0179, 'z', _acute},
    {0x017A, 'z', _acute},
    {0x017B, 'z', _dot_above},
    {0x017C, 'z', _dot_above},
    {0x017D, 'z', _caron},
    {0x017E, 'z', _caron},
    {0x2022, 0x07}, // BULLET
    {0x203C, 0x13}, // DOUBLE EXCLAMATION MARK
    {0x2190, 0x1B}, // LEFTWARDS ARROW
    {0x2191, 0x18}, // UPWARDS ARROW
    {0x2192, 0x1A}, // RIGHTWARDS ARROW
    {0x2193, 0x19}, // DOWNWARDS ARROW
    {0x2194, 0x1D}, // LEFT RIGHT ARROW
    {0x2195, 0x12}, // UP DOWN ARROW
    {0x21A8, 0x17}, // UP DOWN ARROW WITH BASE
    {0x221F, 0x1C}, // RIGHT ANGLE
    {0x2302, 0x7F}, // HOUSE
    {0x25AC, 0x16}, // BLACK RECTANGLE
    {0x25B2, 0x1E}, // BLACK UP-POINTING TRIANGLE
    {0x25BA, 0x10}, // BLACK RIGHT-POINTING POINTER
    {0x25BC, 0x1F}, // BLACK DOWN-POINTING TRIANGLE
    {0x25C4, 0x11}, // BLACK LEFT-POINTING POINTER
    {0x25CB, 0x09}, // WHITE CIRCLE
    {0x25D8, 0x08}, // INVERSE BULLET
    {0x25D9, 0x0A}, // INVERSE WHITE CIRCLE
    {0x263A, 0x01}, // WHITE SMILING FACE
    {0x263B, 0x02}, // BLACK SMILING FACE
    {0x263C, 0x0F}, // WHITE SUN WITH RAYS
    {0x2640, 0x0C}, // FEMALE SIGN
    {0x2642, 0x0B}, // MALE SIGN
    {0x2660, 0x06}, // BLACK SPADE SUIT
    {0x2663, 0x05}, // BLACK CLUB SUIT
    {0x2665, 0x03}, // BLACK HEART SUIT
    {0x2666, 0x04}, // BLACK DIAMOND SUIT
    {0x266A, 0x0D}, // EIGHTH NOTE
    {0x266B, 0x0E}, // BEAMED EIGHTH NOTES
    {0xFFFF}};

char __vid_xfont[8 * sizeof(__vid_font_8x8) / sizeof(gfx_glyph)];
