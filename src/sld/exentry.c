#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <crg.h>
#include <dlg.h>
#include <gfx.h>
#include <pal.h>
#include <sld.h>
#include <snd.h>
#include <vid.h>

extern const char StrSldEnterSerial[];
extern const char StrCrgEncryptedLine1[];
extern const char StrSldSerialLine1[];
extern const char StrSldSerialLine2[];

typedef struct
{
    const void *Data;
    int         DataLength;
    uint32_t    Crc;
    uint32_t   *LongPart;
} SLD_KEY_VALIDATION;

static uint16_t s_Accumulator = 0;

static int
SldExecuteText(SLD_ENTRY *sld);

static int
SldExecuteBitmap(SLD_ENTRY *sld);

static int
SldExecuteRectangle(SLD_ENTRY *sld);

static int
SldExecutePlay(SLD_ENTRY *sld);

static int
SldExecuteScriptCall(SLD_ENTRY *sld);

static hasset
SldFindBestBitmap(char *pattern);

static bool
SldIsXorKeyValid(const uint8_t *key, int keyLength, void *context);

static bool
SldPromptVolumeSerialNumber(char *sn);

static bool
SldIsVolumeSerialNumberValid(const char *sn);

int
SldExecuteEntry(SLD_ENTRY *sld)
{
    pal_sleep(sld->Delay);

    switch (sld->Type)
    {
    case SLD_TYPE_BLANK:
        return 0;
    case SLD_TYPE_LABEL:
        return 0;
    case SLD_TYPE_TEXT:
        return SldExecuteText(sld);
    case SLD_TYPE_BITMAP:
        return SldExecuteBitmap(sld);
    case SLD_TYPE_RECT:
    case SLD_TYPE_RECTF:
        return SldExecuteRectangle(sld);
    case SLD_TYPE_PLAY:
        return SldExecutePlay(sld);
    case SLD_TYPE_WAITKEY:
        s_Accumulator = bios_get_keystroke() >> 8;
        return 0;
    case SLD_TYPE_JUMP:
        return INT_MAX;
    case SLD_TYPE_JUMPE:
        return (s_Accumulator == sld->Vertical) ? INT_MAX : 0;
    case SLD_TYPE_CALL:
        return SldExecuteScriptCall(sld);
    }

    ERR(SLD_UNKNOWN_TYPE);
}

int
SldExecuteText(SLD_ENTRY *sld)
{
    uint16_t x, y = sld->Vertical;

    switch (sld->Horizontal)
    {
    case SLD_ALIGN_CENTER:
        x = (VID_CGA_HIMONO_LINE - sld->Length) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = VID_CGA_HIMONO_LINE - sld->Length;
        break;
    default:
        x = sld->Horizontal;
    }

    return VidDrawText(sld->Content, x, y);
}

int
SldExecuteBitmap(SLD_ENTRY *sld)
{
    int status;

    hasset bitmap = SldFindBestBitmap(sld->Content);
    if (NULL == bitmap)
    {
        ERR(KER_NOT_FOUND);
    }

    char *data = pal_get_asset_data(bitmap);
    if (NULL == data)
    {
        ERR(KER_NOT_FOUND);
    }

    GFX_BITMAP bm;
    status = GfxLoadBitmap(data, &bm);
    if (0 > status)
    {
        return status;
    }

    uint16_t x, y = sld->Vertical;
    switch (sld->Horizontal)
    {
    case SLD_ALIGN_CENTER:
        x = (VID_CGA_HIMONO_LINE - bm.WidthBytes) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = VID_CGA_HIMONO_LINE - bm.WidthBytes;
        break;
    default:
        x = sld->Horizontal;
    }

    status = VidDrawBitmap(&bm, x, y);

    pal_close_asset(bitmap);
    return status;
}

int
SldExecuteRectangle(SLD_ENTRY *sld)
{
    uint16_t x, y = sld->Vertical;
    switch (sld->Horizontal)
    {
    case SLD_ALIGN_CENTER:
        x = (VID_CGA_HIMONO_WIDTH - sld->Shape.Dimensions.Width) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = VID_CGA_HIMONO_HEIGHT - sld->Shape.Dimensions.Height;
        break;
    default:
        x = sld->Horizontal;
    }

    int (*draw)(GFX_DIMENSIONS *, uint16_t, uint16_t, GFX_COLOR);
    draw = (SLD_TYPE_RECT == sld->Type) ? VidDrawRectangle : VidFillRectangle;
    return draw(&sld->Shape.Dimensions, x, y, sld->Shape.Color);
}

int
SldExecutePlay(SLD_ENTRY *sld)
{
    hasset music = pal_open_asset(sld->Content, O_RDONLY);
    if (NULL == music)
    {
        ERR(KER_NOT_FOUND);
    }

    char *data = pal_get_asset_data(music);
    if (NULL == data)
    {
        ERR(KER_NOT_FOUND);
    }

    snd_play(data, pal_get_asset_size(music));
    return 0;
}

int
SldExecuteScriptCall(SLD_ENTRY *sld)
{
    int status;

    hasset script = pal_open_asset(sld->ScriptCall.FileName, O_RDWR);
    if (NULL == script)
    {
        ERR(KER_NOT_FOUND);
    }

    char *data = pal_get_asset_data(script);
    if (NULL == data)
    {
        ERR(KER_NOT_FOUND);
    }

    int size = pal_get_asset_size(script);
    s_Accumulator = 0;

    // Run if stored as plain text
    if (SLD_METHOD_STORE == sld->ScriptCall.Method)
    {
        return SldRunScript(data, size);
    }

    uint8_t            key[sizeof(uint64_t)];
    SLD_KEY_VALIDATION context = {data, size, sld->ScriptCall.Crc32, NULL};
    bool               invalid = false;

    memset(key, 0, sizeof(key));
    switch (sld->ScriptCall.Parameter)
    {
    case SLD_PARAMETER_XOR48_INLINE:
        *(uint64_t *)&key = rstrtoull(sld->ScriptCall.Data, 16);
        break;
    case SLD_PARAMETER_XOR48_PROMPT:
        invalid = !CrgPromptKey(key, 6, 16, SldIsXorKeyValid, &context);
        break;
    case SLD_PARAMETER_XOR48_SPLIT: {
        uint32_t longPart = strtoul(sld->ScriptCall.Data, NULL, 16);
        context.LongPart = &longPart;
        if (!CrgPromptKey(key, 3, 10, SldIsXorKeyValid, &context))
        {
            invalid = true;
            break;
        }

        *(uint64_t *)&key =
            CrgDecodeSplitKey(longPart, *(const uint32_t *)&key);
        context.LongPart = NULL;
        break;
    }
    case SLD_PARAMETER_XOR48_DISKID: {
        uint32_t medium_id = pal_get_medium_id(sld->ScriptCall.Data);
        if (0 == medium_id)
        {
            char sn[10];
            if (!SldPromptVolumeSerialNumber(sn))
            {
                invalid = true;
                break;
            }

            uint32_t highPart = strtoul(sn, NULL, 16);
            uint32_t lowPart = strtoul(sn + 5, NULL, 16);
            medium_id = (highPart << 16) | lowPart;
        }

        context.LongPart = &medium_id;
        if (!CrgPromptKey(key, 3, 10, SldIsXorKeyValid, &context))
        {
            invalid = true;
            break;
        }

        *(uint64_t *)&key =
            CrgDecodeSplitKey(medium_id, *(const uint32_t *)&key);
        context.LongPart = NULL;
        break;
    }
    default:
        invalid = true;
        break;
    }

    // Check the key
    if (invalid || !SldIsXorKeyValid((const uint8_t *)&key, 6, &context))
    {
        s_Accumulator = UINT16_MAX;
        return 0;
    }

    CrgXor(data, data, size, (const uint8_t *)&key, 6);
    sld->ScriptCall.Method = SLD_METHOD_STORE;
    status = SldRunScript(data, size);

    pal_close_asset(script);
    return status;
}

hasset
SldFindBestBitmap(char *pattern)
{
    const char *hex = "0123456789ABCDEF";
    char       *placeholder = strstr(pattern, "<>");
    if (NULL == placeholder)
    {
        return pal_open_asset(pattern, O_RDONLY);
    }

    int par = (int)VidGetPixelAspectRatio();
    int offset = 0;

    while ((0 <= (par + offset)) || (255 >= (par + offset)))
    {
        if ((0 <= (par + offset)) && (255 >= (par + offset)))
        {
            placeholder[0] = hex[(par + offset) / 16];
            placeholder[1] = hex[(par + offset) % 16];
            hasset asset = pal_open_asset(pattern, O_RDONLY);
            if (NULL != asset)
            {
                return asset;
            }
        }

        if (0 <= offset)
        {
            offset++;
        }
        offset = -offset;
    }

    return NULL;
}

bool
SldIsXorKeyValid(const uint8_t *key, int keyLength, void *context)
{
    SLD_KEY_VALIDATION *keyValidation = (SLD_KEY_VALIDATION *)context;

    if (!keyValidation->LongPart)
    {
        return CrgIsXorKeyValid(keyValidation->Data, keyValidation->DataLength,
                                key, keyLength, keyValidation->Crc);
    }

    // 48-bit split key
    uint64_t fullKey =
        CrgDecodeSplitKey(*keyValidation->LongPart, *(const uint32_t *)key);
    return CrgIsXorKeyValid(keyValidation->Data, keyValidation->DataLength,
                            (uint8_t *)&fullKey, 6, keyValidation->Crc);
}

bool
SldPromptVolumeSerialNumber(char *sn)
{
    DlgDrawBackground();

    DLG_FRAME frame = {40, 6};

    DlgDrawFrame(&frame, StrSldEnterSerial);
    DlgDrawText(&frame, StrCrgEncryptedLine1, 0);
    DlgDrawText(&frame, StrSldSerialLine1, 2);
    DlgDrawText(&frame, StrSldSerialLine2, 3);

    int length = DlgInputText(&frame, sn, 9, SldIsVolumeSerialNumberValid, 5);
    if (0 == length)
    {
        return false;
    }

    return true;
}

bool
SldIsVolumeSerialNumberValid(const char *sn)
{
    if (9 != strlen(sn))
    {
        return false;
    }

    for (int i = 0; i < 9; i++)
    {
        if (4 == i)
        {
            if ('-' != sn[i])
            {
                return false;
            }
        }
        else
        {
            if (!isxdigit(sn[i]))
            {
                return false;
            }
        }
    }

    return true;
}
