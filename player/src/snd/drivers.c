#include <snd.h>

typedef int ddcall(drv_init)(void);

extern drv_init __beep_init;
extern drv_init __fluid_init;
extern drv_init __mme_init;
extern drv_init __mpu401_init;
extern drv_init __opl2_init;

static drv_init *const INBOX_INIT[] = {
#if defined(CONFIG_SOUND_FLUID)
    __fluid_init, // FluidSynth
#endif
#if defined(CONFIG_SOUND_WINMM)
    __mme_init, // Windows MME
#endif
#if defined(CONFIG_SOUND_MPU401)
    __mpu401_init, // Roland MPU-401 UART
#endif
#if defined(CONFIG_SOUND_OPL2)
    __opl2_init, // Yamaha YM3812 (OPL2)
#endif
#if defined(CONFIG_SOUND_BEEP)
    __beep_init, // PC Speaker
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
