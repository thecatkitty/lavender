#include <base.h>

#if defined(__WATCOMC__)
uint16_t
BSWAP16(uint16_t x)
{
    return ((x & 0x00FFU) << 8) | ((x & 0xFF00U) >> 8);
}

uint32_t
BSWAP32(uint32_t x)
{
    return ((x & 0x000000FFUL) << 24) | ((x & 0x0000FF00UL) << 8) |
           ((x & 0x00FF0000UL) >> 8) | ((x & 0xFF000000UL) >> 24);
}
#endif
