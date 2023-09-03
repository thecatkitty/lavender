#include <stdio.h>

#include <fmt/wave.h>
#include <pal.h>

#define PCSPK_RATE 44100

uint16_t _divisor = 0;
uint32_t _counter = 0;
bool     _enabled = false;
uint32_t _last = 0;

FILE           *_wav = NULL;
iff_head        _ch_root;
iff_head        _ch_fmt;
iff_head        _ch_data;
long            _ch_data_pos;
wave_pcm_format _format;

static void
_flush(void)
{
    if (!_wav)
    {
        _ch_root.type.dw = IFF_FOURCC_RIFF.dw;

        _ch_fmt.type.dw = WAVE_FOURCC_FMT.dw;
        _format.wf.format = WAVE_FORMAT_PCM;
        _format.wf.channels = 1;
        _format.wf.sample_rate = PCSPK_RATE;
        _format.wf.byte_rate = PCSPK_RATE;
        _format.wf.block_align = _format.wf.byte_rate;
        _format.bits_per_sample = 8;
        _ch_fmt.length = sizeof(_format);

        _ch_data.type.dw = WAVE_FOURCC_DATA.dw;
        _ch_data.length = 0;

        _ch_root.length = sizeof(WAVE_FOURCC_WAVE) + sizeof(_ch_fmt) +
                          _ch_fmt.length + sizeof(_ch_data) + _ch_data.length;

        _wav = fopen("speaker.wav", "wb");
        fwrite(&_ch_root, sizeof(_ch_root), 1, _wav);
        fwrite(&WAVE_FOURCC_WAVE, sizeof(WAVE_FOURCC_WAVE), 1, _wav);

        fwrite(&_ch_fmt, sizeof(_ch_fmt), 1, _wav);
        fwrite(&_format, _ch_fmt.length, 1, _wav);

        _ch_data_pos = ftell(_wav);
        fwrite(&_ch_data, sizeof(_ch_data), 1, _wav);
        _wav = freopen(NULL, "rb+", _wav);
    }

    fseek(_wav, 0, SEEK_END);

    uint32_t now = pal_get_counter();
    uint32_t length = (now - _last) * PCSPK_RATE / 1000;
    uint32_t cycle = PCSPK_RATE * _divisor / 1193182;
    if (_enabled)
    {
        for (int i = 0; i < length; i++, _counter++)
        {
            fputc(128 + ((_counter % cycle) < (cycle / 2) ? -96 : +96), _wav);
        }
    }
    else
    {
        for (int i = 0; i < length; i++, _counter++)
        {
            fputc(128, _wav);
        }
    }

    _ch_root.length += length;
    fseek(_wav, 0, SEEK_SET);
    fwrite(&_ch_root, sizeof(_ch_root), 1, _wav);

    _ch_data.length += length;
    fseek(_wav, _ch_data_pos, SEEK_SET);
    fwrite(&_ch_data, sizeof(_ch_data), 1, _wav);

    fseek(_wav, 0, SEEK_END);
    _last = now;
}

void
pcspkemu_stop(void)
{
    LOG("entry");

    if (_wav)
    {
        _flush();
        fclose(_wav);
        _wav = NULL;
    }
}

void
dospc_beep(uint16_t divisor)
{
    _flush();
    _divisor = divisor;
    _enabled = true;
}

void
dospc_silence(void)
{
    _flush();
    _enabled = false;
}
