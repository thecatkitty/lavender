
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <crg.h>
#include <dlg.h>

extern const char StrCrgEnterPassword[];
extern const char StrCrgEncryptedLine1[];
extern const char StrCrgEncryptedLine2[];
extern const char StrCrgKeyInvalid[];

static bool
IsHexString(const char *str);

bool
CrgPromptKey(uint8_t *         key,
             int               keyLength,
             int               base,
             CRG_KEY_VALIDATOR validate,
             void *            context)
{
    DlgDrawBackground();

    char *    buffer = (char *)alloca(keyLength * 3 + 1);
    DLG_FRAME frame = {36, 4};

    while (true)
    {
        DlgDrawFrame(&frame, StrCrgEnterPassword);
        DlgDrawText(&frame, StrCrgEncryptedLine1, 0);
        DlgDrawText(&frame, StrCrgEncryptedLine2, 1);

        int length = DlgInputText(&frame, buffer, keyLength * 2,
                                  (16 == base) ? IsHexString : NULL, 3);
        if (0 == length)
        {
            return false;
        }

        if (0 == base)
        {
            memcpy(key, buffer, length);
        }
        else
        {
            uint64_t value = strtoull(buffer, NULL, base);
            memcpy(key, &value, keyLength);
        }

        if (!validate)
        {
            return true;
        }

        if (validate(key, keyLength, context))
        {
            return true;
        }

        DlgDrawFrame(&frame, StrCrgEnterPassword);
        DlgDrawText(&frame, StrCrgKeyInvalid, 1);
        BiosKeyboardGetKeystroke();
    }
}

bool
IsHexString(const char *str)
{
    if (!*str)
    {
        return false;
    }

    while (*str)
    {
        if (!isxdigit(*str))
        {
            return false;
        }
        str++;
    }

    return true;
}
