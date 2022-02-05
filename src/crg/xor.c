#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <api/dos.h>
#include <crg.h>
#include <dlg.h>
#include <ker.h>

typedef struct
{
    const uint8_t *Data;
    const uint8_t *Key;
    int            KeyLength;
} XOR_STREAM;

static uint8_t
XorGetByte(void *context, int i);

static bool
IsHexString(const char *str);

bool
CrgIsXorKeyValid(const void *   data,
                 int            dataLength,
                 const uint8_t *key,
                 int            keyLength,
                 uint32_t       crc)
{
    XOR_STREAM stream;
    stream.Data = (const uint8_t *)data;
    stream.Key = key;
    stream.KeyLength = keyLength;

    return crc == KerCalculateZipCrcIndirect(XorGetByte, &stream, dataLength);
}

void
CrgXor(const void *   src,
       void *         dst,
       int            dataLength,
       const uint8_t *key,
       int            keyLength)
{
    const uint8_t *source = (const uint8_t *)src;
    uint8_t *      destination = (uint8_t *)dst;

    for (int i = 0; i < dataLength; i++)
    {
        destination[i] = source[i] ^ key[i % keyLength];
    }
}

extern const char StrCrgEnterPassword[];
extern const char StrCrgEncryptedLine1[];
extern const char StrCrgEncryptedLine2[];

bool
CrgPromptKey(uint8_t *key, int keyLength, int base)
{
    DlgDrawBackground();

    char *    buffer = (char *)alloca(keyLength * 3 + 1);
    DLG_FRAME frame = {36, 4};
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

    return true;
}

static uint8_t
XorGetByte(void *context, int i)
{
    XOR_STREAM *stream = (XOR_STREAM *)context;

    return stream->Data[i] ^ stream->Key[i % stream->KeyLength];
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
