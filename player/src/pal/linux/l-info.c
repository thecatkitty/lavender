#include <arch/linux.h>
#include <pal.h>

#include "impl.h"

#if defined(CONFIG_SOUND_BEEPEMU)
bool linux_has_beepemu = false;

bool
linux_beepemu_enabled(void)
{
    LOG(linux_has_beepemu ? "yes" : "no");
    return linux_has_beepemu;
}
#endif
