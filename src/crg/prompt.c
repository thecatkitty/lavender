
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <crg.h>
#include <dlg.h>

extern const char StrCrgEnterPassword[];
extern const char StrCrgEncrypted[];
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
    char *buffer = (char *)alloca(keyLength * 3 + 1);
    while (true)
    {
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

        int length = dlg_prompt(StrCrgEnterPassword, StrCrgEncrypted, buffer,
                                keyLength * 2, inputValidator);
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
            uint64_t value = rstrtoull(buffer, base);
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

        dlg_alert(StrCrgEnterPassword, StrCrgKeyInvalid);
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
