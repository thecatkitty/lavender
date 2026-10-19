#ifndef _ARCH_LINUX_H_
#define _ARCH_LINUX_H_

#include <base.h>

#if defined(CONFIG_SOUND_BEEPEMU)
extern bool
linux_beepemu_enabled(void);
#endif

#endif
