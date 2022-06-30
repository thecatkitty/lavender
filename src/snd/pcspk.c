#include <pal/dospc.h>
#include <snd.h>

void
snd_beep(uint16_t divisor)
{
    dospc_beep(divisor);
}

void
snd_silence(void)
{
    dospc_silence();
}
