
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
AllCharacters(const char *str, bool (*predicate)(int));

static bool
IsDecString(const char *str);

static bool
IsHexString(const char *str);

bool
CrgPromptKey(uint8_t          *key,
             int               keyLength,
             int               base,
             CRG_KEY_VALIDATOR validate,
             void             *context)
{
    DlgDrawBackground();

    char     *buffer = (char *)alloca(keyLength * 3 + 1);
    DLG_FRAME frame = {36, 5};

    while (true)
    {
        DlgDrawFrame(&frame, StrCrgEnterPassword);
        DlgDrawText(&frame, StrCrgEncryptedLine1, 0);
        DlgDrawText(&frame, StrCrgEncryptedLine2, 2);

        bool (*inputValidator)(const char *) = NULL;
        switch (base)
        {
        case 10:
            inputValidator = IsDecString;
            break;
        case 16:
            inputValidator = IsHexString;
            break;
        }

        int length =
            DlgInputText(&frame, buffer, keyLength * 2, inputValidator, 4);
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
        bios_get_keystroke();
    }
}

bool
AllCharacters(const char *str, bool (*predicate)(int))
{
    if (!*str)
    {
        return false;
    }

    while (*str)
    {
        if (!predicate(*str))
        {
            return false;
        }
        str++;
    }

    return true;
}

bool
IsHexString(const char *str)
{
    return AllCharacters(str, (bool (*)(int))isxdigit);
}

bool
IsDecString(const char *str)
{
    return AllCharacters(str, (bool (*)(int))isdigit);
}
