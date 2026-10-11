#include <windows.h>

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

#if defined(__GNUC__) && (WINVER <= 0x0400) && (_M_IX86 < 400) &&              \
    !defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_4)
/* InterlockedCompareExchange, which was only added in Windows 98, is required
 * by MinGW-w64 CRT initialization on i386 due to a lack of support for the
 * cmpxchg instruction in older CPUs. This implementation uses a simple spinlock
 * to ensure atomicity.
 *
 * See __tmainCRTStartup in mingw-w64-crt/crt/crtexe.c for more details.
 */

static volatile LONG icmpxchg_lock_ = 0;

static LONG __stdcall
icmpxchg_impl_(LONG volatile *Destination, LONG Exchange, LONG Comperand)
{
    while (InterlockedExchange(&icmpxchg_lock_, 1) != 0)
    {
        Sleep(0);
    }

    LONG old = *Destination;

    if (old == Comperand)
    {
        *Destination = Exchange;
    }

    icmpxchg_lock_ = 0;

    return old;
}

LONG(__stdcall *const __imp__InterlockedCompareExchange)(
    volatile LONG *, LONG, LONG) __asm__("__imp__InterlockedCompareExchange@12")
    __attribute__((used)) = icmpxchg_impl_;
#endif
