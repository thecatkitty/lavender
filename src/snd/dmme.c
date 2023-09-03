#include <pal.h>
#include <snd.h>

bool
snd_initialize(void)
{
    LOG("entry");

    return false;
}

void
snd_cleanup(void)
{
    LOG("entry");
}

void
snd_send(midi_event *event)
{
}
