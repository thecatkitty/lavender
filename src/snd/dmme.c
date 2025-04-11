#include <stdio.h>
#include <windows.h>

#include <pal.h>
#include <snd/dev.h>

static HMIDIOUT _out = NULL;

#if PAL_EXTERNAL_TICK
static unsigned _period = 0;
static UINT     _timer = 0;

static void CALLBACK
_time_callback(UINT id, UINT msg, DWORD_PTR user, DWORD_PTR dw1, DWORD_PTR dw2)
{
    snd_tick();
}
#endif

static bool ddcall
mme_open(device *dev)
{
#if PAL_EXTERNAL_TICK
    TIMECAPS tc;
    unsigned period;
#endif

    LOG("entry");

#if PAL_EXTERNAL_TICK
    if (TIMERR_NOERROR != timeGetDevCaps(&tc, sizeof(TIMECAPS)))
    {
        LOG("cannot get multimedia timer capabilities");
        return false;
    }
#endif

    if (MMSYSERR_NOERROR !=
        midiOutOpen(&_out, (UINT)(UINT_PTR)dev->data, 0, 0, CALLBACK_NULL))
    {
        LOG("cannot open MIDI device");
        return false;
    }

#if PAL_EXTERNAL_TICK
    period = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
    if (TIMERR_NOERROR != timeBeginPeriod(period))
    {
        LOG("cannot request the multimedia timer period");
        snd_device_close(dev);
        return false;
    }

    _period = period;

    if (0 ==
        (_timer = timeSetEvent(_period, 0, _time_callback, 0, TIME_PERIODIC)))
    {
        LOG("cannot start the multimedia timer");
        snd_device_close(dev);
        return false;
    }
#endif

    return true;
}

static void ddcall
mme_close(device *dev)
{
    LOG("entry");

#if PAL_EXTERNAL_TICK
    if (_timer)
    {
        timeKillEvent(_timer);
    }

    if (_period)
    {
        timeEndPeriod(_period);
    }
#endif

    if (_out)
    {
        midiOutClose(_out);
    }
}

static bool ddcall
mme_write(device *dev, const midi_event *event)
{
    if (MIDI_MSG_SYSEX <= event->status)
    {
        return true;
    }

    if (0 == event->msg_length)
    {
        if (MMSYSERR_NOERROR != midiOutShortMsg(_out, event->status))
        {
            LOG("error sending message %02x", event->status);
            return false;
        }

        return true;
    }

    if (1 == event->msg_length)
    {
        char msg[sizeof(DWORD)] = {(char)event->status, event->msg[0]};
        if (MMSYSERR_NOERROR != midiOutShortMsg(_out, *(DWORD *)msg))
        {
            LOG("error sending message %02x %02x", msg[0], msg[1]);
            return false;
        }

        return true;
    }

    if (2 == event->msg_length)
    {
        char msg[sizeof(DWORD)] = {(char)event->status, event->msg[0],
                                   event->msg[1]};
        if (MMSYSERR_NOERROR != midiOutShortMsg(_out, *(DWORD *)msg))
        {
            LOG("error sending message %02x %02x %02x", msg[0], msg[1], msg[2]);
            return false;
        }

        return true;
    }

    LOG("cannot send %d-byte %02x message", event->msg_length + 1,
        event->status);
    return true;
}

static snd_device_ops _ops = {mme_open, mme_close, mme_write};

int
__mme_init(void)
{
    UINT devices, id;

    devices = midiOutGetNumDevs();
    if (0 == devices)
    {
        LOG("no MIDI output devices");
        return ENODEV;
    }

    for (id = 0; id < devices; id++)
    {
        MIDIOUTCAPSW caps;
        device       dev;

        if (MMSYSERR_NOERROR != midiOutGetDevCapsW(id, &caps, sizeof(caps)))
        {
            LOG("out %u: cannot get capabilities", id);
            continue;
        }

        LOG("out %u: %04x:%04x %u.%u %ls", id, caps.wMid, caps.wPid,
            HIBYTE(caps.vDriverVersion), LOBYTE(caps.vDriverVersion),
            caps.szPname);

        snprintf(dev.name, DEV_MAX_NAME, "mme%u", id);
        WideCharToMultiByte(CP_UTF8, 0, caps.szPname, -1, dev.description, 32,
                            NULL, NULL);
        dev.ops = &_ops;
        dev.data = (void *)(UINT_PTR)id;
        snd_register_device(&dev);
    }

    return 0;
}
