#include <pal.h>
#include <snd.h>

extern snd_format_protocol __snd_fmidi;
extern snd_format_protocol __snd_fspk;

static snd_format_protocol *_formats[] = {
    &__snd_fmidi, // Standard MIDI File, type 0
    &__snd_fspk   // length-divisor pairs for PC Speaker
};

#define MAX_FORMATS (sizeof(_formats) / sizeof(snd_format_protocol *))

static snd_format_protocol _format;
static hasset              _music = NULL;

void
snd_handle(void)
{
    if (_format.step && _music)
    {
        if (!_format.step())
        {
            pal_close_asset(_music);
            _music = NULL;
        }
    }
}

bool
snd_play(const char *name)
{
    _format.probe = 0;
    _format.start = 0;
    _format.step = 0;

    if (_music)
    {
        pal_close_asset(_music);
    }

    _music = pal_open_asset(name, O_RDONLY);
    if (NULL == _music)
    {
        return false;
    }

    char *data = pal_get_asset_data(_music);
    if (NULL == data)
    {
        return false;
    }

    int length = pal_get_asset_size(_music);
    for (snd_format_protocol **format = _formats;
         format < _formats + MAX_FORMATS; format++)
    {
        if ((*format)->probe(data, length))
        {
            _format = **format;
            break;
        }
    }

    if (_format.start)
    {
        _format.start(data, length);
        return true;
    }

    pal_close_asset(_music);
    _music = NULL;
    return false;
}
