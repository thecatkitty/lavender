#include <stdlib.h>
#include <string.h>

#include <crg.h>
#include <fmt/zip.h>

#include "crg_impl.h"
#include "des.h"

typedef struct
{
    uint64_t subkeys[DES_ROUNDS];
    size_t   at_pos;
    uint64_t at_pt;
} des_context;
#define CONTEXT(stream) ((des_context *)(stream->_context))

static uint64_t
_permute(const char *table,
         uint8_t     table_len,
         uint64_t    input,
         uint8_t     input_len)
{
    uint64_t ret = 0;
    for (uint8_t i = 0; i < table_len; i++)
    {
        ret |= ((input >> (input_len - table[i])) & 1) << (table_len - i - 1);
    }
    return ret;
}

static uint32_t
_rotl28(uint32_t value, uint8_t count)
{
    return ((value << count) & 0xFFFFFFF) | (value >> (28 - count));
}

static uint64_t
_from_bytes(const uint8_t *block)
{
    return __builtin_bswap64(*(const uint64_t *)block);
}

static void
_to_bytes(uint64_t num, uint8_t *block)
{
    *(uint64_t *)block = __builtin_bswap64(num);
}

static void
_generate_subkeys(uint64_t key, uint64_t subkeys[])
{
    // Permute key using PC1
    uint64_t K_56 = _permute(PC1, lengthof(PC1), key, 64);

    // Split into 28-bit halves
    uint32_t C_28 = (uint32_t)((K_56 >> 28) & 0xFFFFFFF);
    uint32_t D_28 = (uint32_t)(K_56 & 0xFFFFFFF);

    for (uint8_t round = 0; round < DES_ROUNDS; round++)
    {
        C_28 = _rotl28(C_28, ROTATIONS[round]);
        D_28 = _rotl28(D_28, ROTATIONS[round]);

        uint64_t CD_56 = ((uint64_t)C_28 << 28) | (uint64_t)D_28;
        subkeys[round] = _permute(PC2, lengthof(PC2), CD_56, 56);
    }
}

static char
_S(int sbox, uint8_t input)
{
    char row = (char)(((input & 0x20) >> 4) | (input & 1));
    char col = (char)((input & 0x1E) >> 1);
    return SBOXES[sbox][SBOX_XY(row, col)];
}

static uint32_t
_F(uint64_t K, uint32_t R)
{
    // Expand R to 48 bits and xor with K
    uint64_t e = _permute(E, lengthof(E), R, 32) ^ K;

    // Apply substitution boxes, compress to 32 bits
    uint32_t output = 0;
    for (int i = 0; i < 8; i++)
    {
        output <<= 4;
        output |= (uint32_t)_S(i, (uint8_t)((e & 0xFC0000000000) >> 42));
        e <<= 6;
    }

    // Permute substitution box output
    return (uint32_t)_permute(P, lengthof(P), output, 32);
}

static uint64_t
_des(const uint64_t subkeys[], uint64_t M)
{
    // Initial permutation of data
    M = _permute(IP, lengthof(IP), M, 64);

    // Split into two 32-bit halves
    uint32_t L = (uint32_t)(M >> 32) & 0xFFFFFFFF;
    uint32_t R = (uint32_t)(M & 0xFFFFFFFF);

    // Substitute
    for (int i = 0; i < DES_ROUNDS; i++)
    {
        uint32_t L_old = L;
        uint64_t subkey = subkeys[DES_ROUNDS - i - 1];
        L = R;
        R = L_old ^ _F(subkey, R);
    }

    // Combine swapped halves
    M = (((uint64_t)R) << 32) | (uint64_t)L;

    // Final permutation
    return _permute(FP, lengthof(FP), M, 64);
}

static bool
des_allocate(crg_stream *stream)
{
    if (sizeof(uint64_t) != stream->key_length)
    {
        return false;
    }

    if (stream->data_length & 7)
    {
        return false;
    }

    if ((2 * sizeof(uint64_t)) > stream->data_length)
    {
        return false;
    }

    stream->_context = malloc(sizeof(des_context));
    if (NULL == stream->_context)
    {
        return false;
    }

    CONTEXT(stream)->at_pos = 0;
    CONTEXT(stream)->at_pt = 0;

    _generate_subkeys(_from_bytes(stream->key), CONTEXT(stream)->subkeys);
    return true;
}

static bool
des_free(crg_stream *stream)
{
    if (NULL == stream->_context)
    {
        return false;
    }

    free(stream->_context);
    stream->_context = NULL;
    return true;
}

static uint8_t
des_at(crg_stream *stream, size_t i)
{
    size_t position = sizeof(uint64_t) + (i & ~7);
    if (CONTEXT(stream)->at_pos != position)
    {
        uint64_t iv = _from_bytes(stream->data + position - sizeof(uint64_t));
        uint64_t ct = _from_bytes(stream->data + position);
        CONTEXT(stream)->at_pos = position;
        CONTEXT(stream)->at_pt = _des(CONTEXT(stream)->subkeys, ct) ^ iv;
    }

    const uint8_t *bytes = (const uint8_t *)&CONTEXT(stream)->at_pt;
    return bytes[7 - (i & 7)];
}

static bool
des_decrypt(crg_stream *stream, uint8_t *dst)
{
    size_t length = stream->data_length - sizeof(uint64_t);

    // Load initialization vector
    uint64_t       ct_previous = _from_bytes(stream->data);
    const uint8_t *ct_ptr = stream->data + sizeof(uint64_t);
    const uint8_t *ct_padding = stream->data + length;

    // Process the unpadded part of the ciphertext
    while (ct_ptr < ct_padding)
    {
        uint64_t ct = _from_bytes(ct_ptr);
        uint64_t pt = _des(CONTEXT(stream)->subkeys, ct) ^ ct_previous;
        _to_bytes(pt, dst);
        ct_previous = ct;

        ct_ptr += sizeof(uint64_t);
        dst += sizeof(uint64_t);
    }

    // Process the padded part of the ciphertext
    uint64_t ct = _from_bytes(ct_ptr);
    uint64_t pt = _des(CONTEXT(stream)->subkeys, ct) ^ ct_previous;

    uint8_t pt_bytes[sizeof(uint64_t)];
    _to_bytes(pt, pt_bytes);
    uint8_t padding = pt_bytes[7];
    memcpy(dst, pt_bytes, 8 - padding);

    // Store the real data length
    stream->data_length = length - padding;
    return true;
}

static bool
des_validate(crg_stream *stream, uint32_t crc)
{
    size_t   position = stream->data_length - sizeof(uint64_t);
    uint64_t iv = _from_bytes(stream->data + position - sizeof(uint64_t));
    uint64_t ct = _from_bytes(stream->data + position);
    uint64_t pt = _des(CONTEXT(stream)->subkeys, ct) ^ iv;

    uint8_t pt_bytes[sizeof(uint64_t)];
    _to_bytes(pt, pt_bytes);
    uint8_t padding = pt_bytes[7];

    size_t length = stream->data_length - sizeof(uint64_t) - padding;
    return crc == zip_calculate_crc_indirect((uint8_t(*)(void *, size_t))des_at,
                                             stream, length);
}

crg_stream_impl __crg_des_impl = {.allocate = des_allocate,
                                  .free = des_free,
                                  .at = des_at,
                                  .decrypt = des_decrypt,
                                  .validate = des_validate};
