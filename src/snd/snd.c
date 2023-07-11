#include <snd.h>

extern snd_format_protocol __snd_fspk;

static snd_format_protocol *_formats[] = {
    &__snd_fspk // length-divisor pairs for PC Speaker
};

#define MAX_FORMATS (sizeof(_formats) / sizeof(snd_format_protocol *))

static snd_format_protocol _format;

void
snd_handle(void)
{
    if (_format.step)
    {
        _format.step();
    }
}

void
snd_play(void *music, uint16_t length)
{
    _format.probe = 0;
    _format.start = 0;
    _format.step = 0;

    for (snd_format_protocol **format = _formats;
         format < _formats + MAX_FORMATS; format++)
    {
        if ((*format)->probe(music, length))
        {
            _format = **format;
            break;
        }
    }

    if (_format.start)
    {
        _format.start(music, length);
    }
}
