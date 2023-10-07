#include <crg.h>

#define UINTN_MAX(n) ((1UL << n) - 1)

#define LE32B6D_KEYLO      0
#define LE32B6D_KEYLO_MASK UINTN_MAX(24)
#define LE32B6D_KEYHI      24
#define LE32B6D_KEYHI_MASK UINTN_MAX(24)

#define LE32B6D_LOCHI_ROTATION      30
#define LE32B6D_LOCHI_ROTATION_MASK UINTN_MAX(2)
#define LE32B6D_LOCHI               24
#define LE32B6D_LOCHI_MASK          UINTN_MAX(6)
#define LE32B6D_LOCLO               0
#define LE32B6D_LOCLO_MASK          UINTN_MAX(24)

#define LE32B6D_EXTLO_ROTATION      18
#define LE32B6D_EXTLO_ROTATION_MASK UINTN_MAX(2)
#define LE32B6D_EXTHI               0
#define LE32B6D_EXTHI_MASK          UINTN_MAX(18)
#define LE32B6D_EXTHI_SHIFT         6

static const unsigned LE32B6D_ROTATIONS[] = {3, 7, 13, 19};

static uint32_t
_rotr24(uint32_t n, unsigned c)
{
    return (n >> c | n << (24 - c)) & UINTN_MAX(24);
}

static uint64_t
_decode_le32b6d(const void *src)
{
    uint32_t local = ((const uint32_t *)src)[0];
    uint32_t external = ((const uint32_t *)src)[1];

    uint32_t low = (local >> LE32B6D_LOCLO) & LE32B6D_LOCLO_MASK;
    uint32_t high = (((external >> LE32B6D_EXTHI) & LE32B6D_EXTHI_MASK)
                     << LE32B6D_EXTHI_SHIFT) |
                    ((local >> LE32B6D_LOCHI) & LE32B6D_LOCHI_MASK);

    unsigned rlow =
        (external >> LE32B6D_EXTLO_ROTATION) & LE32B6D_EXTLO_ROTATION_MASK;
    unsigned rhigh =
        (local >> LE32B6D_LOCHI_ROTATION) & LE32B6D_LOCHI_ROTATION_MASK;

    low = _rotr24(low, LE32B6D_ROTATIONS[rlow]);
    high = _rotr24(high, LE32B6D_ROTATIONS[rhigh]);

    return (((uint64_t)high & LE32B6D_KEYHI_MASK) << LE32B6D_KEYHI) |
           (((uint64_t)low & LE32B6D_KEYLO_MASK) << LE32B6D_KEYLO);
}

uint64_t
crg_decode_key(const void *src, crg_keysm sm)
{
    errno = 0;

    if (sm == CRG_KEYSM_LE32B6D)
    {
        return _decode_le32b6d(src);
    }

    errno = EFTYPE;
    return 0;
}
