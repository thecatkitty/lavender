#include <libi86/string.h>

#include <api/bios.h>
#include <api/dos.h>
#include <dev/cga.h>
#include <ker.h>
#include <vid.h>

#define VID_PAR(dx, dy, sx, sy)                                                \
    (uint8_t)(64U * (unsigned)dy * (unsigned)sx / (unsigned)dx / (unsigned)sy)

extern char                     abExtendedFont[];
extern far char *               lpabPreviousFont;
extern VID_CHARACTER_DESCRIPTOR astFontData[];

static int
VesaReadEdid(EDID *edid);

uint16_t
VidSetMode(uint16_t mode)
{
    uint16_t previous = BiosVideoGetMode() & 0xFF;
    BiosVideoSetMode((uint8_t)mode);
    return previous;
}

uint16_t
VidGetPixelAspectRatio(void)
{
    const uint8_t ratios[4] = {
        VID_PAR(16, 10, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT),
        VID_PAR(4, 3, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT),
        VID_PAR(5, 4, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT),
        VID_PAR(16, 9, VID_CGA_HIMONO_WIDTH, VID_CGA_HIMONO_HEIGHT)};

    EDID edid;
    if (0 > VesaReadEdid(&edid))
    {
        return ratios[EDID_TIMING_ASPECT_4_3];
    }

    return ratios[edid.StandardTiming[0] >> EDID_TIMING_ASPECT];
}

int
VidDrawBitmap(GFX_BITMAP *bm, uint16_t x, uint16_t y)
{
    if ((1 != bm->Planes) || (1 != bm->BitsPerPixel))
    {
        ERR(VID_FORMAT);
    }

    far void *bits = bm->Bits;
    far void *plane0 = MK_FP(CGA_HIMONO_MEM, 0);
    far void *plane1 = MK_FP(CGA_HIMONO_MEM, CGA_HIMONO_PLANE);
    plane0 += x + y * (VID_CGA_HIMONO_LINE / 2);
    plane1 += x + y * (VID_CGA_HIMONO_LINE / 2);

    for (uint16_t line = 0; line < bm->Height; line += 2)
    {
        _fmemcpy(plane0, bits, bm->WidthBytes);
        plane0 += VID_CGA_HIMONO_LINE;
        bits += bm->WidthBytes;

        _fmemcpy(plane1, bits, bm->WidthBytes);
        plane1 += VID_CGA_HIMONO_LINE;
        bits += bm->WidthBytes;
    }
}

int
VidDrawText(const char *str, uint16_t x, uint16_t y)
{
    BiosVideoSetCursorPosition(0, (y << 8) | x);
    while (*str)
    {
        DosPutC(*str);
        str++;
    }
}

void
VidLoadFont(void)
{
    lpabPreviousFont =
        (char *)KerInstallIsr((isr)abExtendedFont, INT_CGA_EXTENDED_FONT_PTR);

    far const char *bfont =
        MK_FP(CGA_BASIC_FONT_SEGMENT, CGA_BASIC_FONT_OFFSET);
    char *                    xfont = abExtendedFont;
    VID_CHARACTER_DESCRIPTOR *fdata = astFontData;

    if (KerIsDosBox())
    {
        xfont++; // DOSBox ROM font is moved one line to the bottom
    }

    while (0xFFFF != fdata->CodePoint)
    {
        if (NULL == fdata->Overlay)
        {
            fdata++;
            continue;
        }

        if (0 != fdata->Base)
        {
            _fmemcpy(xfont, bfont + 8 * fdata->Base, 8);
        }

        unsigned ovheight = fdata->Overlay[0] & 0xF;
        unsigned ovtop = fdata->Overlay[0] >> 4;
        for (unsigned i = 1; i <= ovheight; i++)
        {
            xfont[ovtop + i - 1] |= fdata->Overlay[i];
        }

        fdata++;
        xfont += 8;
    }
}

void
VidUnloadFont(void)
{
    KerUninstallIsr((isr)lpabPreviousFont, INT_CGA_EXTENDED_FONT_PTR);
}

int
VesaReadEdid(EDID *edid)
{
    short ax;

    ax = BiosVideoVbeDcCapabilities();
    if (0x4F != (ax & 0xFF))
    {
        ERR(VID_UNSUPPORTED);
    }

    if (0 != (ax & 0xFF00))
    {
        ERR(VID_FAILED);
    }

    ax = BiosVideoVbeDcReadEdid(edid);
    if (0x4F != (ax & 0xFF))
    {
        ERR(VID_UNSUPPORTED);
    }

    if (0 != (ax & 0xFF00))
    {
        ERR(VID_UNSUPPORTED);
    }

    return 0;
}
