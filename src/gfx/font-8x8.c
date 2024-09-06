#include "glyph.h"

#define OVERLAY(name, top, height, ...)                                        \
    [name] = {(((top) << 4) | ((height) & 0xF)), __VA_ARGS__}

#define TRANSFORMATION(name, ...) [name] = {__VA_ARGS__, GXF_END}

// Character overlays
enum overlay
{
    ACUTE = 1,
    DOT_ABOVE,
    OGONEK,
    STROKE,
    TILDE,
    DIAERESIS,
    CARON,
    RING,
    APOSTROPHE,
};

const uint8_t DRV_RDAT __gfx_overlays[][1 + GFX_MAX_OVERLAY_SIZE] = {
    OVERLAY(ACUTE, 0, 1, 0x1C),              //
    OVERLAY(DOT_ABOVE, 0, 1, 0x30),          //
    OVERLAY(OGONEK, 6, 2, 0x0E, 0x07),       //
    OVERLAY(STROKE, 2, 3, 0x04, 0x78, 0x80), //
    OVERLAY(TILDE, 0, 1, 0x7C),              //
    OVERLAY(DIAERESIS, 0, 1, 0xCC),          //
    OVERLAY(CARON, 0, 2, 0x6C, 0x38),        //
    OVERLAY(RING, 0, 2, 0x30, 0x30),         //
    OVERLAY(APOSTROPHE, 0, 1, 0x03),         //
};

// Character transformations
enum transformation
{
    MOVE_TWO_TOP = 1,
    SQUISH_UPPER_A,
    SQUISH_UPPER_E,
    SQUISH_UPPER_N,
    SQUISH_UPPER_Y,
};

const uint8_t DRV_RDAT __gfx_xforms[][GFX_MAX_TRANSFORMATION_SIZE] = {
    TRANSFORMATION(MOVE_TWO_TOP, //
                   GXF_SELECT(0),
                   GXF_GROW(1),
                   GXF_MOVE(2)),
    TRANSFORMATION(SQUISH_UPPER_A, //
                   GXF_SELECT(0),
                   GXF_GROW(4),
                   GXF_MOVE(1),
                   GXF_SELECT(1),
                   GXF_GROW(1),
                   GXF_MOVE(1)),
    TRANSFORMATION(SQUISH_UPPER_E, //
                   GXF_SELECT(0),
                   GXF_GROW(1),
                   GXF_MOVE(1),
                   GXF_SELECT(1),
                   GXF_GROW(2),
                   GXF_MOVE(1)),
    TRANSFORMATION(SQUISH_UPPER_N, //
                   GXF_SELECT(1),
                   GXF_GROW(3),
                   GXF_MOVE(1),
                   GXF_CLEAR(0)),
    TRANSFORMATION(SQUISH_UPPER_Y, //
                   GXF_SELECT(1),
                   GXF_GROW(2),
                   GXF_MOVE(1),
                   GXF_CLEAR(0)),
};

// Character descriptors
const gfx_glyph DRV_RDAT __gfx_font_8x8[] = {
    {0x00A7, 0x15}, // SECTION SIGN
    {0x00B6, 0x14}, // PILCROW SIGN
    {0x00C1, 'A', ACUTE, SQUISH_UPPER_A},
    {0x00C9, 'E', ACUTE, SQUISH_UPPER_E},
    {0x00CD, 'I', ACUTE, MOVE_TWO_TOP},
    {0x00D1, 'N', TILDE, SQUISH_UPPER_N},
    {0x00D3, 'O', ACUTE, MOVE_TWO_TOP},
    {0x00DA, 'U', ACUTE, MOVE_TWO_TOP},
    {0x00DC, 'U', DIAERESIS, MOVE_TWO_TOP},
    {0x00DD, 'Y', ACUTE, SQUISH_UPPER_Y},
    {0x00E1, 'a', ACUTE},
    {0x00E9, 'e', ACUTE},
    {0x00ED, 'i', ACUTE},
    {0x00F1, 'n', TILDE},
    {0x00F3, 'o', ACUTE},
    {0x00FA, 'u', ACUTE},
    {0x00FC, 'u', DIAERESIS},
    {0x00FD, 'y', ACUTE},
    {0x0104, 'A', OGONEK},
    {0x0105, 'a', OGONEK},
    {0x0106, 'C', ACUTE, MOVE_TWO_TOP},
    {0x0107, 'c', ACUTE},
    {0x010C, 'C', CARON, MOVE_TWO_TOP},
    {0x010D, 'c', CARON},
    {0x010E, 'D', CARON, MOVE_TWO_TOP},
    {0x010F, 'd', APOSTROPHE},
    {0x0118, 'E', OGONEK},
    {0x0119, 'e', OGONEK},
    {0x011A, 'E', CARON, SQUISH_UPPER_E},
    {0x011B, 'e', CARON},
    {0x0141, 'L', STROKE},
    {0x0142, 'l', STROKE},
    {0x0143, 'N', ACUTE, SQUISH_UPPER_N},
    {0x0144, 'n', ACUTE},
    {0x0147, 'N', CARON, SQUISH_UPPER_N},
    {0x0148, 'n', CARON},
    {0x0158, 'R', CARON, MOVE_TWO_TOP},
    {0x0159, 'r', CARON},
    {0x015A, 's', ACUTE},
    {0x015B, 's', ACUTE},
    {0x0160, 's', CARON},
    {0x0161, 's', CARON},
    {0x0164, 'T', CARON, MOVE_TWO_TOP},
    {0x0165, 't', APOSTROPHE},
    {0x016E, 'U', RING, MOVE_TWO_TOP},
    {0x016F, 'u', RING},
    {0x0179, 'z', ACUTE},
    {0x017A, 'z', ACUTE},
    {0x017B, 'z', DOT_ABOVE},
    {0x017C, 'z', DOT_ABOVE},
    {0x017D, 'z', CARON},
    {0x017E, 'z', CARON},
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
