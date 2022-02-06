#include <crg.h>

#define UINTN_MAX(n) ((1UL << n) - 1)

#define KEY_LOW_PART       0
#define KEY_LOW_PART_MASK  UINTN_MAX(24)
#define KEY_HIGH_PART      24
#define KEY_HIGH_PART_MASK UINTN_MAX(24)

#define LONGPART_HIGH_ROTATION      30
#define LONGPART_HIGH_ROTATION_MASK UINTN_MAX(2)
#define LONGPART_HIGH_PART          24
#define LONGPART_HIGH_PART_MASK     UINTN_MAX(6)
#define LONGPART_LOW_PART           0
#define LONGPART_LOW_PART_MASK      UINTN_MAX(24)

#define SHORTPART_LOW_ROTATION      18
#define SHORTPART_LOW_ROTATION_MASK UINTN_MAX(2)
#define SHORTPART_HIGH_PART         0
#define SHORTPART_HIGH_PART_MASK    UINTN_MAX(18)
#define SHORTPART_HIGH_PART_SHIFT   6

static const unsigned KeyRotationOffsets[] = {3, 7, 13, 19};

static uint32_t
RotateRight24(uint32_t n, unsigned c)
{
    return (n >> c | n << (24 - c)) & UINTN_MAX(24);
}

uint64_t
CrgDecodeSplitKey(uint32_t longPart, uint32_t shortPart)
{
    uint32_t lowPart = (longPart >> LONGPART_LOW_PART) & LONGPART_LOW_PART_MASK;
    uint32_t highPart =
        (((shortPart >> SHORTPART_HIGH_PART) & SHORTPART_HIGH_PART_MASK)
         << SHORTPART_HIGH_PART_SHIFT) |
        ((longPart >> LONGPART_HIGH_PART) & LONGPART_HIGH_PART_MASK);

    unsigned lowRotation =
        (shortPart >> SHORTPART_LOW_ROTATION) & SHORTPART_LOW_ROTATION_MASK;
    unsigned highRotation =
        (longPart >> LONGPART_HIGH_ROTATION) & LONGPART_HIGH_ROTATION_MASK;

    lowPart = RotateRight24(lowPart, KeyRotationOffsets[lowRotation]);
    highPart = RotateRight24(highPart, KeyRotationOffsets[highRotation]);

    return (((uint64_t)highPart & KEY_HIGH_PART_MASK) << KEY_HIGH_PART) |
           (((uint64_t)lowPart & KEY_LOW_PART_MASK) << KEY_LOW_PART);
}
