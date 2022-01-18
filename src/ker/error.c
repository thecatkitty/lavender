#include <api/dos.h>
#include <ker.h>

extern const char __serrf[];
extern const char __serrm[];

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
    DosPutS(ErrFindMessage(__serrf, facility));
    DosPutS(" - $");
    DosPutS(ErrFindMessage(__serrm, error));

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
