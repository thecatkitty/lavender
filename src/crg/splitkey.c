#include <crg.h>

#define UINTN_MAX(n) ((1UL << n) - 1)

#define KEY_LOW       0
#define KEY_LOW_MASK  UINTN_MAX(24)
#define KEY_HIGH      24
#define KEY_HIGH_MASK UINTN_MAX(24)

#define LOCAL_HIGH_ROTATION      30
#define LOCAL_HIGH_ROTATION_MASK UINTN_MAX(2)
#define LOCAL_HIGH               24
#define LOCAL_HIGH_MASK          UINTN_MAX(6)
#define LOCAL_LOW                0
#define LOCAL_LOW_MASK           UINTN_MAX(24)

#define EXTERNAL_LOW_ROTATION      18
#define EXTERNAL_LOW_ROTATION_MASK UINTN_MAX(2)
#define EXTERNAL_HIGH              0
#define EXTERNAL_HIGH_MASK         UINTN_MAX(18)
#define EXTERNAL_HIGH_SHIFT        6

static const unsigned _rotation_offsets[] = {3, 7, 13, 19};

static uint32_t
_rotr24(uint32_t n, unsigned c)
{
    return (n >> c | n << (24 - c)) & UINTN_MAX(24);
}

uint64_t
crg_combine_key(uint32_t local, uint32_t external)
{
    uint32_t low = (local >> LOCAL_LOW) & LOCAL_LOW_MASK;
    uint32_t high = (((external >> EXTERNAL_HIGH) & EXTERNAL_HIGH_MASK)
                     << EXTERNAL_HIGH_SHIFT) |
                    ((local >> LOCAL_HIGH) & LOCAL_HIGH_MASK);

    unsigned rlow =
        (external >> EXTERNAL_LOW_ROTATION) & EXTERNAL_LOW_ROTATION_MASK;
    unsigned rhigh = (local >> LOCAL_HIGH_ROTATION) & LOCAL_HIGH_ROTATION_MASK;

    low = _rotr24(low, _rotation_offsets[rlow]);
    high = _rotr24(high, _rotation_offsets[rhigh]);

    return (((uint64_t)high & KEY_HIGH_MASK) << KEY_HIGH) |
           (((uint64_t)low & KEY_LOW_MASK) << KEY_LOW);
}
