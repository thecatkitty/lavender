#include <crg.h>
#include <ker.h>

typedef struct
{
    const uint8_t *Data;
    const uint8_t *Key;
    int            KeyLength;
} XOR_STREAM;

static uint8_t
XorGetByte(void *context, int i);

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

static uint8_t
XorGetByte(void *context, int i)
{
    XOR_STREAM *stream = (XOR_STREAM *)context;

    return stream->Data[i] ^ stream->Key[i % stream->KeyLength];
}
