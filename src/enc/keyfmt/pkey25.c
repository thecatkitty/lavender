#include <enc.h>

#include "../enc_impl.h"

#define PKEY25_BASE   24
#define PKEY25_LENGTH 25

#define PKEY25XOR12_UDATA_LENGTH 13
#define PKEY25XOR12_EKEY_LENGTH  12

#define PKEY25XOR2B_K1E_LENGTH 13 // X[1..0] K2[55] K1[55..0]
#define PKEY25XOR2B_K2_LENGTH  12 // K2[54..0]

static const char PKEY25_ALPHABET[] = "2346789BCDFGHJKMPQRTVWXY";

static const uint64_t PKEY25XOR2B_MASK[] = {
    0x00000000000000ULL,
    0x55555555555555ULL,
    0xAAAAAAAAAAAAAAULL,
    0xFFFFFFFFFFFFFFULL,
};

static void
_remove_hyphens(const char *src, char *dst)
{
    char *ptr = dst;
    while (ptr < dst + PKEY25_LENGTH)
    {
        if ('-' != *src)
        {
            *ptr = *src;
            ptr++;
        }
        src++;
    }
}

static uint64_t
_aatoull(const char *str, size_t length, const char *alphabet, size_t base)
{
    uint64_t ret = 0;

    const char *end;
    for (end = str + length; str < end; str++)
    {
        const char *pos = strchr(alphabet, *str);
        if (NULL == pos)
        {
            return 0;
        }

        ret *= base;
        ret += pos - alphabet;
    }
    return ret;
}

int
__enc_pkey25raw_decode(const void *src, void *dst)
{
    const char *left_part, *right_part;

    // Remove hyphens
    char pkey[PKEY25_LENGTH];
    _remove_hyphens((const char *)src, pkey);

    // Convert to integer parts
    left_part = pkey;
    right_part = pkey + PKEY25XOR12_UDATA_LENGTH;

    ((uint64_t *)dst)[0] = _aatoull(left_part, PKEY25XOR12_UDATA_LENGTH,
                                    PKEY25_ALPHABET, PKEY25_BASE);
    ((uint64_t *)dst)[1] = _aatoull(right_part, PKEY25XOR12_EKEY_LENGTH,
                                    PKEY25_ALPHABET, PKEY25_BASE);
    return 0;
}

int
__enc_pkey25xor12_decode(const void *src, void *dst)
{
    const char *left_part, *right_part;
    uint64_t    udata, ekey;

    // Remove hyphens
    char pkey[PKEY25_LENGTH];
    _remove_hyphens((const char *)src, pkey);

    // Convert to integer parts
    left_part = pkey;
    right_part = pkey + PKEY25XOR12_UDATA_LENGTH;
    udata = _aatoull(left_part, PKEY25XOR12_UDATA_LENGTH, PKEY25_ALPHABET,
                     PKEY25_BASE);
    ekey = _aatoull(right_part, PKEY25XOR12_EKEY_LENGTH, PKEY25_ALPHABET,
                    PKEY25_BASE);

    // Retrieve the 64-bit key
    __enc_des_expand56(ekey ^ udata, (uint8_t *)dst);
    return 0;
}

int
__enc_pkey25xor2b_decode(const void *src, void *dst)
{
    const char *left_part, *right_part;
    uint64_t    k1e, k2, x;

    // Remove hyphens
    char pkey[PKEY25_LENGTH];
    _remove_hyphens((const char *)src, pkey);

    // Convert to integer parts
    left_part = pkey;
    right_part = pkey + PKEY25XOR2B_K1E_LENGTH;
    k1e = _aatoull(left_part, PKEY25XOR2B_K1E_LENGTH, PKEY25_ALPHABET,
                   PKEY25_BASE);
    k2 = _aatoull(right_part, PKEY25XOR2B_K2_LENGTH, PKEY25_ALPHABET,
                  PKEY25_BASE);

    // Retrieve the 64-bit key
    x = PKEY25XOR2B_MASK[(k1e >> 57) & 3];
    k2 |= (k1e >> 1) & (1ULL << 55);
    k1e &= (1ULL << 56) - 1;

    __enc_des_expand56(k1e ^ x, (uint8_t *)dst);
    __enc_des_expand56(k2 ^ x, (uint8_t *)dst + sizeof(uint64_t));
    return 0;
}

bool
__enc_pkey25_validate_format(const char *key)
{
    size_t i;
    for (i = 0; i < PKEY25_LENGTH + 4; i++)
    {
        if (5 == (i % 6))
        {
            if ('-' != key[i])
            {
                return false;
            }
        }
        else if ((0 == key[i]) || (NULL == strchr(PKEY25_ALPHABET, key[i])))
        {
            return false;
        }
    }

    return PKEY25_LENGTH + 4 == i;
}
