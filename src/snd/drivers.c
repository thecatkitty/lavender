#include <snd.h>

typedef int ddcall(drv_init)(void);

extern drv_init __beep_init;
extern drv_init __fluid_init;
extern drv_init __mme_init;
extern drv_init __mpu401_init;
extern drv_init __opl2_init;

static drv_init *const INBOX_INIT[] = {
#if defined(__ia16__) && !defined(CONFIG_ANDREA)
    __mpu401_init, // Roland MPU-401 UART
    __opl2_init,   // Yamaha YM3812 (OPL2)
#endif

#if defined(__ia16__)
    __beep_init, // PC Speaker
#endif

#if defined(__linux__)
    __fluid_init, // FluidSynth
#endif

#if defined(_WIN32)
    __mme_init, // Windows MME
#endif
};

void
snd_load_inbox_drivers(void)
{
    int i;

    for (i = 0; i < lengthof(INBOX_INIT); i++)
    {
        INBOX_INIT[i]();
    }
}
