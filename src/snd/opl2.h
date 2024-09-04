#include <drv.h>

#pragma pack(push, 1)
typedef struct
{
    uint8_t car_wave_select : 2;
    uint8_t mod_wave_select : 2;
    uint8_t feedback_conn_reg : 4;

    uint8_t car_level;
    uint8_t car_amvib_reg;
    uint8_t car_decay : 4;
    uint8_t car_attack : 4;
    uint8_t car_release : 4;
    uint8_t car_sustain : 4;

    uint8_t mod_level;
    uint8_t mod_amvib_reg;
    uint8_t mod_decay : 4;
    uint8_t mod_attack : 4;
    uint8_t mod_release : 4;
    uint8_t mod_sustain : 4;
} opl2_patch;
#pragma pack(pop)

extern const opl2_patch DRV_DATA __snd_drums_opl2[5];
extern const opl2_patch DRV_DATA __snd_gm_opl2[128];
