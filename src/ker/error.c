#include <api/dos.h>
#include <ker.h>

extern const char ErrFacilities[];
extern const char ErrMessages[];

extern void
PitDeinitialize(void);

char KerLastError;

static const char *
ErrFindMessage(const char *messages, unsigned key);

void
KerTerminate(void)
{
    unsigned error = (unsigned char)KerLastError;
    unsigned facility = error >> 5;

    DosPutS("ERROR: $");
    DosPutS(ErrFindMessage(ErrFacilities, facility));
    DosPutS(" - $");
    DosPutS(ErrFindMessage(ErrMessages, error));

    PitDeinitialize();
    DosExit(KerLastError);
}

// Find a message using its key byte
//   WILL CRASH IF MESSAGE NOT FOUND!
const char *
ErrFindMessage(const char *messages, unsigned key)
{
    while (true)
    {
        if (key == *messages)
        {
            return messages + 1;
        }

        while ('$' != *messages)
        {
            messages++;
        }

        messages++;
    }
}
