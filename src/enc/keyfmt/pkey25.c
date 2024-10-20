#include <enc.h>

static uint64_t
_aatoull(const char *str, size_t length, const char *alphabet, size_t base)
{
    uint64_t ret = 0;
    for (const char *end = str + length; str < end; str++)
    {
        ret *= base;
        const char *pos = strchr(alphabet, *str);
        if (NULL == pos)
        {
            return 0;
        }

        ret += pos - alphabet;
    }
    return ret;
}

uint64_t
__enc_pkey25xor12_decode(const void *src)
{
    const char *left_part = (const char *)src;
    const char *right_part = (const char *)src + PKEY25XOR12_UDATA_LENGTH;

    uint64_t udata =
        _aatoull(left_part, PKEY25XOR12_UDATA_LENGTH, PKEY25XOR12_ALPHABET, 24);
    uint64_t ekey =
        _aatoull(right_part, PKEY25XOR12_EKEY_LENGTH, PKEY25XOR12_ALPHABET, 24);
    uint64_t key_56 = ekey ^ udata;

    uint8_t key[sizeof(uint64_t)];
    for (size_t b = 0; b < lengthof(key); b++)
    {
        key[b] = ((uint8_t)key_56 & 0x7F) << 1;
        if (!__builtin_parity(key[b]))
        {
            key[b] |= 1;
        }
        key_56 >>= 7;
    }

    return *(uint64_t *)key;
}
