#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <crg.h>
#include <gfx.h>
#include <ker.h>
#include <sld.h>
#include <vid.h>

static uint16_t Accumulator = 0;

static int
SldExecuteText(SLD_ENTRY *sld);

static int
SldExecuteBitmap(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip);

static int
SldExecuteRectangle(SLD_ENTRY *sld);

static int
SldExecutePlay(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip);

static int
SldExecuteScriptCall(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip);

static int
SldFindBestBitmap(char *                  pattern,
                  unsigned                length,
                  ZIP_CDIR_END_HEADER *   zip,
                  ZIP_LOCAL_FILE_HEADER **lfh);

int
SldExecuteEntry(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip)
{
    KerSleep(sld->Delay);

    switch (sld->Type)
    {
    case SLD_TYPE_BLANK:
        return 0;
    case SLD_TYPE_LABEL:
        return 0;
    case SLD_TYPE_TEXT:
        return SldExecuteText(sld);
    case SLD_TYPE_BITMAP:
        return SldExecuteBitmap(sld, zip);
    case SLD_TYPE_RECT:
    case SLD_TYPE_RECTF:
        return SldExecuteRectangle(sld);
    case SLD_TYPE_PLAY:
        return SldExecutePlay(sld, zip);
    case SLD_TYPE_WAITKEY:
        Accumulator = BiosKeyboardGetKeystroke() >> 8;
        return 0;
    case SLD_TYPE_JUMP:
        return INT_MAX;
    case SLD_TYPE_JUMPE:
        return (Accumulator == sld->Vertical) ? INT_MAX : 0;
    case SLD_TYPE_CALL:
        return SldExecuteScriptCall(sld, zip);
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
SldExecuteBitmap(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip)
{
    int status;

    ZIP_LOCAL_FILE_HEADER *lfh;
    status = SldFindBestBitmap(sld->Content, sld->Length, zip, &lfh);
    if (0 > status)
    {
        return status;
    }

    void *data;
    status = KerGetArchiveData(lfh, &data);
    if (0 > status)
    {
        return status;
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

    return VidDrawBitmap(&bm, x, y);
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
SldExecutePlay(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip)
{
    int status;

    ZIP_LOCAL_FILE_HEADER *lfh;
    status = KerSearchArchive(zip, sld->Content, sld->Length, &lfh);
    if (0 > status)
    {
        return status;
    }

    void *data;
    status = KerGetArchiveData(lfh, &data);
    if (0 > status)
    {
        return status;
    }

    KerStartPlayer(data, lfh->UncompressedSize);
    return 0;
}

int
SldExecuteScriptCall(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip)
{
    int status;

    ZIP_LOCAL_FILE_HEADER *lfh;
    status = KerSearchArchive(zip, sld->ScriptCall.FileName, sld->Length, &lfh);
    if (0 > status)
    {
        return status;
    }

    void *data;
    status = KerGetArchiveData(lfh, &data);
    if (0 > status)
    {
        return status;
    }

    Accumulator = 0;

    // Run if stored as plain text
    if (SLD_METHOD_STORE == sld->ScriptCall.Method)
    {
        return SldRunScript(data, lfh->UncompressedSize, zip);
    }

    // Run if already decrypted
    if (lfh->Crc32 == sld->ScriptCall.Crc32)
    {
        return SldRunScript(data, lfh->UncompressedSize, zip);
    }

    uint8_t key[48 / 8];
    switch (sld->ScriptCall.Parameter)
    {
    case SLD_PARAMETER_XOR48_INLINE:
        *(uint64_t *)&key = strtoull(sld->ScriptCall.Data, NULL, 16);
        break;
    case SLD_PARAMETER_XOR48_PROMPT:
        if (!CrgPromptKey(key, sizeof(key), 16))
        {
            Accumulator = UINT16_MAX;
            return 0;
        }
        break;
    default:
        Accumulator = UINT16_MAX;
        return 0;
    }

    // Check the key
    if (!CrgIsXorKeyValid(data, lfh->UncompressedSize, (const uint8_t *)&key, 6,
                          sld->ScriptCall.Crc32))
    {
        Accumulator = UINT16_MAX;
        return 0;
    }

    CrgXor(data, data, lfh->UncompressedSize, (const uint8_t *)&key, 6);
    lfh->Crc32 = KerCalculateZipCrc((uint8_t *)data, lfh->UncompressedSize);
    return SldRunScript(data, lfh->UncompressedSize, zip);
}

int
SldFindBestBitmap(char *                  pattern,
                  unsigned                length,
                  ZIP_CDIR_END_HEADER *   zip,
                  ZIP_LOCAL_FILE_HEADER **lfh)
{
    const char *hex = "0123456789ABCDEF";
    char *      placeholder = strstr(pattern, "<>");
    if (NULL == placeholder)
    {
        return KerSearchArchive(zip, pattern, length, lfh);
    }

    int par = (int)VidGetPixelAspectRatio();
    int offset = 0;

    while ((0 <= (par + offset)) || (255 >= (par + offset)))
    {
        if ((0 <= (par + offset)) && (255 >= (par + offset)))
        {
            placeholder[0] = hex[(par + offset) / 16];
            placeholder[1] = hex[(par + offset) % 16];
            if (0 == KerSearchArchive(zip, pattern, length, lfh))
            {
                return 0;
            }
        }

        if (0 <= offset)
        {
            offset++;
        }
        offset = -offset;
    }

    ERR(KER_NOT_FOUND);
}
