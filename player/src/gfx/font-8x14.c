#include "glyph.h"

#define OVERLAY(name, top, height, ...)                                        \
    [name] = {(((top) << 4) | ((height) & 0xF)), __VA_ARGS__}

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
    ACUTE_LOW,
    DOT_ABOVE_LOW,
    TILDE_LOW,
    DIAERESIS_LOW,
    CARON_LOW,
    RING_LOW,
};

const uint8_t DRV_RDAT __gfx_overlays[][1 + GFX_MAX_OVERLAY_SIZE] = {
    OVERLAY(ACUTE, 0, 2, 0x1C, 0x38),            //
    OVERLAY(DOT_ABOVE, 0, 2, 0x38, 0x38),        //
    OVERLAY(OGONEK, 12, 2, 0x0E, 0x07),          //
    OVERLAY(STROKE, 6, 3, 0x04, 0x78, 0x80),     //
    OVERLAY(TILDE, 0, 2, 0x76, 0xDC),            //
    OVERLAY(DIAERESIS, 0, 2, 0xC6, 0xC6),        //
    OVERLAY(CARON, 0, 2, 0x6C, 0x38),            //
    OVERLAY(RING, 0, 3, 0x10, 0x28, 0x10),       //
    OVERLAY(APOSTROPHE, 2, 3, 0x03, 0x01, 0x02), //
    OVERLAY(ACUTE_LOW, 3, 2, 0x1C, 0x38),        //
    OVERLAY(DOT_ABOVE_LOW, 3, 2, 0x38, 0x38),    //
    OVERLAY(TILDE_LOW, 3, 2, 0x76, 0xDC),        //
    OVERLAY(DIAERESIS_LOW, 3, 2, 0xCC, 0xCC),    //
    OVERLAY(CARON_LOW, 3, 2, 0x6C, 0x38),        //
    OVERLAY(RING_LOW, 3, 3, 0x10, 0x28, 0x10),   //
};

// Character descriptors
const gfx_glyph DRV_RDAT __gfx_font_8x14[] = {
    {0x00A7, 0x15}, // SECTION SIGN
    {0x00B6, 0x14}, // PILCROW SIGN
    {0x00C1, 'A', ACUTE},
    {0x00C9, 'E', ACUTE},
    {0x00CD, 'I', ACUTE},
    {0x00D1, 'N', TILDE},
    {0x00D3, 'O', ACUTE},
    {0x00DA, 'U', ACUTE},
    {0x00DC, 'U', DIAERESIS},
    {0x00DD, 'Y', ACUTE},
    {0x00E1, 'a', ACUTE_LOW},
    {0x00E9, 'e', ACUTE_LOW},
    {0x00ED, 'i', ACUTE_LOW},
    {0x00F1, 'n', TILDE_LOW},
    {0x00F3, 'o', ACUTE_LOW},
    {0x00FA, 'u', ACUTE_LOW},
    {0x00FC, 'u', DIAERESIS_LOW},
    {0x00FD, 'y', ACUTE_LOW},
    {0x0104, 'A', OGONEK},
    {0x0105, 'a', OGONEK},
    {0x0106, 'C', ACUTE},
    {0x0107, 'c', ACUTE_LOW},
    {0x010C, 'C', CARON},
    {0x010D, 'c', CARON_LOW},
    {0x010E, 'D', CARON},
    {0x010F, 'd', APOSTROPHE},
    {0x0118, 'E', OGONEK},
    {0x0119, 'e', OGONEK},
    {0x011A, 'E', CARON},
    {0x011B, 'e', CARON_LOW},
    {0x0141, 'L', STROKE},
    {0x0142, 'l', STROKE},
    {0x0143, 'N', ACUTE},
    {0x0144, 'n', ACUTE_LOW},
    {0x0147, 'N', CARON},
    {0x0148, 'n', CARON_LOW},
    {0x0158, 'R', CARON},
    {0x0159, 'r', CARON_LOW},
    {0x015A, 'S', ACUTE},
    {0x015B, 's', ACUTE_LOW},
    {0x0160, 'S', CARON},
    {0x0161, 's', CARON_LOW},
    {0x0164, 'T', CARON},
    {0x0165, 't', APOSTROPHE},
    {0x016E, 'U', RING},
    {0x016F, 'u', RING_LOW},
    {0x0179, 'Z', ACUTE},
    {0x017A, 'z', ACUTE_LOW},
    {0x017B, 'Z', DOT_ABOVE},
    {0x017C, 'z', DOT_ABOVE_LOW},
    {0x017D, 'Z', CARON},
    {0x017E, 'z', CARON_LOW},
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
