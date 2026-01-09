#include <arch/windows.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900)
#pragma warning(disable : 28159)
#endif

static uint16_t version_ = 0;

uint16_t
windows_get_version(void)
{
    return version_ ? version_ : (version_ = BSWAP16(LOWORD(GetVersion())));
}
