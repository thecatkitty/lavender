#include <string.h>

#include <gfx.h>
#include <ker.h>
#include <sld.h>
#include <vid.h>

static int
SldExecuteText(SLD_ENTRY *sld);

static int
SldExecuteBitmap(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip);

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
    case SLD_TYPE_TEXT:
        return SldExecuteText(sld);
    case SLD_TYPE_BITMAP:
        return SldExecuteBitmap(sld, zip);
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
        x = (SLD_ENTRY_MAX_LENGTH - sld->Length) / 2;
        break;
    case SLD_ALIGN_RIGHT:
        x = SLD_ENTRY_MAX_LENGTH - sld->Length;
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
