#include <libi86/string.h>

#include <api/dos.h>
#include <ker.h>

extern DOS_PSP *KerPsp;

bool
KerIsDosBox(void)
{
    const char far *str = (const char far *)0xF000E061;
    for (const char *i = "DOSBox"; *i; i++, str++)
    {
        if (*i != *str)
            return false;
    }
    return true;
}

bool
KerIsDosMajor(uint8_t major)
{
    uint8_t dosMajor = DosGetVersion() & 0xFF;
    return (1 == major) ? (0 == dosMajor) : (major == dosMajor);
}

bool
KerIsWindowsNt(void)
{
    // NTVDM claims to be DOS 5
    if (!KerIsDosMajor(5))
    {
        return false; // It's not 5, so it's not NTVDM.
    }

    // Does it have %OS% environment variable?
    far const char *os = KerGetEnvironmentVariable("OS");
    if (0 == FP_SEG(os))
    {
        return false; // It's just a regular DOS 5.
    }

    // Does it claim to be Windows NT?
    const char windowsNt[] = "Windows_NT";
    return 0 == _fmemcmp(os, windowsNt, sizeof(windowsNt));
}

far const char *
KerGetEnvironmentVariable(const char *key)
{
    far const char *env = MK_FP(KerPsp->EnvironmentSegment, 0);
    size_t          keyLength = strlen(key);

    while (*env)
    {
        if ((0 == _fmemcmp(key, env, keyLength)) && ('=' == env[keyLength]))
        {
            return env + keyLength + 1;
        }

        env += _fstrlen(env) + 1;
    }

    return MK_FP(0, 0);
}
