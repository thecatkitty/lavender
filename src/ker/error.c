#include <api/dos.h>
#include <err.h>
#include <ker.h>

extern const char __serrf[];
extern const char __serrm[];

extern void
PitDeinitialize(void);

static const char *
FindMessage(const char *messages, unsigned key);

extern const char StrKerError[];

void
KerTerminate(int error)
{
    unsigned facility = error >> 5;

    DosPutS(StrKerError);
    DosPutS(FindMessage(__serrf, facility));
    DosPutS(" - $");
    DosPutS(FindMessage(__serrm, error));

    DosExit(error);
}

// Find a message using its key byte
//   WILL CRASH IF MESSAGE NOT FOUND!
const char *
FindMessage(const char *messages, unsigned key)
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
