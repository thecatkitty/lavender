#include <enc.h>

// PKEY25XOR12 definitions
#define PKEY25XOR12_BASE         24
#define PKEY25XOR12_UDATA_LENGTH 13
#define PKEY25XOR12_EKEY_LENGTH  12
#define PKEY25XOR12_LENGTH       (PKEY25XOR12_UDATA_LENGTH + PKEY25XOR12_EKEY_LENGTH)

static const char PKEY25XOR12_ALPHABET[] = "2346789BCDFGHJKMPQRTVWXY";

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

static uint8_t
_parity(uint8_t n)
{
#if defined(_MSC_VER)
    uint8_t ret = 0;

    while (n)
    {
        ret ^= (n & 1);
        n >>= 1;
    }

    return ret;
#else
    return __builtin_parity(n);
#endif
}

int
__enc_pkey25xor12_decode(const void *src, void *dst)
{
    const char *str = (const char *)src;
    uint8_t    *key = (uint8_t *)dst;

    // Remove hyphens
    char  pkey[PKEY25XOR12_LENGTH];
    char *ppkey = pkey;
    while (ppkey < pkey + PKEY25XOR12_LENGTH)
    {
        if ('-' != *str)
        {
            *ppkey = *str;
            ppkey++;
        }
        str++;
    }

    // Convert to integer parts
    {
        const char *left_part = pkey;
        const char *right_part = pkey + PKEY25XOR12_UDATA_LENGTH;
        uint64_t    udata = _aatoull(left_part, PKEY25XOR12_UDATA_LENGTH,
                                     PKEY25XOR12_ALPHABET, 24);
        uint64_t    ekey = _aatoull(right_part, PKEY25XOR12_EKEY_LENGTH,
                                    PKEY25XOR12_ALPHABET, 24);

        // Retrieve the 64-bit key
        uint64_t key_56 = ekey ^ udata;
        size_t   b;
        for (b = 0; b < sizeof(uint64_t); b++)
        {
            key[b] = ((uint8_t)key_56 & 0x7F) << 1;
            if (!_parity(key[b]))
            {
                key[b] |= 1;
            }
            key_56 >>= 7;
        }
    }

    return 0;
}

bool
__enc_pkey25xor12_validate_format(const char *key)
{
    size_t i;
    for (i = 0; i < PKEY25XOR12_LENGTH + 4; i++)
    {
        if (5 == (i % 6))
        {
            if ('-' != key[i])
            {
                return false;
            }
        }
        else if ((0 == key[i]) ||
                 (NULL == strchr(PKEY25XOR12_ALPHABET, key[i])))
        {
            return false;
        }
    }

    return PKEY25XOR12_LENGTH + 4 == i;
}
