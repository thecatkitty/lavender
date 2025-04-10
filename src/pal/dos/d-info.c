#include <arch/dos/winoldap.h>
#include <base.h>

#ifdef CONFIG_ANDREA
#include <andrea.h>
#endif

#ifdef CONFIG_IA16X
static short winoldap_version = 0;
#endif

bool ddcall
dos_is_dosbox(void)
{
    return 0 == _fmemcmp((const char far *)0xF000E061, "DOSBox", 6);
}

bool
dos_is_windows(void)
{
#ifdef CONFIG_IA16X
    if (0 == winoldap_version)
    {
        winoldap_version = winoldap_get_version();
    }

    return 0x1700 != winoldap_version;
#else
    return false;
#endif
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(dos_is_dosbox);
#endif
