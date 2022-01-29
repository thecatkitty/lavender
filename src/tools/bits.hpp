#pragma once

#include <climits>
#include <cstdint>

inline constexpr uint64_t
umaxval(unsigned bits)
{
    return (1ULL << bits) - 1;
}

template <typename T>
inline constexpr T
rotl(T n, unsigned c, unsigned l = sizeof(T) * CHAR_BIT)
{
    return (n << c | n >> (l - c)) & umaxval(l);
}

template <typename T>
inline constexpr T
rotr(T n, unsigned c, unsigned l = sizeof(T) * CHAR_BIT)
{
    return (n >> c | n << (l - c)) & umaxval(l);
}
