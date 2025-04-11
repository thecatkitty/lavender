#include <arch/windows.h>

static uint16_t version_ = 0;

uint16_t
windows_get_version(void)
{
    return version_ ? version_ : (version_ = BSWAP16(LOWORD(GetVersion())));
}
