#include <windows.h>

#include <pal.h>
#include <snd.h>

static HMIDIOUT _out = NULL;

bool
snd_initialize(void)
{
    LOG("entry");

    UINT devices = midiOutGetNumDevs();
    if (0 == devices)
    {
        LOG("no MIDI output devices");
        return false;
    }

    for (UINT id = 0; id < devices; id++)
    {
        MIDIOUTCAPSW caps;

        if (MMSYSERR_NOERROR != midiOutGetDevCapsW(id, &caps, sizeof(caps)))
        {
            LOG("out %u: cannot get capabilities", id);
            continue;
        }

        LOG("out %u: %04x:%04x %u.%u %ls", id, caps.wMid, caps.wPid,
            HIBYTE(caps.vDriverVersion), LOBYTE(caps.vDriverVersion),
            caps.szPname);
    }

    if (MMSYSERR_NOERROR !=
        midiOutOpen(&_out, MIDI_MAPPER, 0, 0, CALLBACK_NULL))
    {
        LOG("cannot open MIDI mapper device");
        return false;
    }

    return true;
}

void
snd_cleanup(void)
{
    LOG("entry");

    if (_out)
    {
        midiOutClose(_out);
    }
}

void
snd_send(midi_event *event)
{
    if (MIDI_MSG_SYSEX <= event->status)
    {
        return;
    }

    if (0 == event->msg_length)
    {
        if (MMSYSERR_NOERROR != midiOutShortMsg(_out, event->status))
        {
            LOG("error sending message %02x", event->status);
        }
        return;
    }

    if (1 == event->msg_length)
    {
        char msg[sizeof(DWORD)] = {(char)event->status, event->msg[0]};
        if (MMSYSERR_NOERROR != midiOutShortMsg(_out, *(DWORD *)msg))
        {
            LOG("error sending message %02x %02x", msg[0], msg[1]);
        }
        return;
    }

    if (2 == event->msg_length)
    {
        char msg[sizeof(DWORD)] = {(char)event->status, event->msg[0],
                                   event->msg[1]};
        if (MMSYSERR_NOERROR != midiOutShortMsg(_out, *(DWORD *)msg))
        {
            LOG("error sending message %02x %02x %02x", msg[0], msg[1], msg[2]);
        }
        return;
    }

    LOG("cannot send %d-byte %02x message", event->msg_length + 1,
        event->status);
}
