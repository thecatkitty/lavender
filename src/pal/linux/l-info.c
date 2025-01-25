#include <arch/linux.h>
#include <pal.h>

#include "impl.h"

bool linux_has_beepemu = false;

bool
linux_beepemu_enabled(void)
{
    LOG(linux_has_beepemu ? "yes" : "no");
    return linux_has_beepemu;
}
