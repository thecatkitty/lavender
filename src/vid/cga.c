#include <libi86/string.h>

#include <api/bios.h>
#include <api/dos.h>
#include <dev/cga.h>
#include <ker.h>
#include <vid.h>

#define VID_PAR(dx, dy, sx, sy)                                                \
    (uint8_t)(64U * (unsigned)dy * (unsigned)sx / (unsigned)dx / (unsigned)sy)
#define VID_CGA_HIMONO_XY(x, y) ((x) / 8 + (y / 2) * VID_CGA_HIMONO_LINE)

extern char                     __VidExtendedFont[];
extern VID_CHARACTER_DESCRIPTOR __VidFontData[];

static isr       PreviousFontPtr;
static far void *CgaPlane0 = MK_FP(CGA_HIMONO_MEM, 0);
static far void *CgaPlane1 = MK_FP(CGA_HIMONO_MEM, CGA_HIMONO_PLANE);

static void
FontExecuteGlyphTransformation(const char *gxf, char *glyph);

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
    far void *plane0 = CgaPlane0;
    far void *plane1 = CgaPlane1;
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
VidDrawRectangle(GFX_DIMENSIONS *rect, uint16_t x, uint16_t y, GFX_COLOR color)
{
    uint16_t left = x - 1;
    uint16_t right = x + rect->Width;
    uint16_t top = y - 1;
    uint16_t bottom = y + rect->Height;

    far char *plane;
    uint8_t   leftCorner = (1 << (8 - (left % 8))) - 1;
    uint8_t   rightCorner = ~((1 << (7 - (right % 8))) - 1);
    uint8_t   leftBorder = 1 << (7 - (left % 8));
    uint8_t   rightBorder = 1 << (7 - (right % 8));
    uint8_t   horizontalFill = (GFX_COLOR_WHITE == color) ? 0xFF : 0x00;

    // Top line
    plane = (far uint8_t *)((top % 2) ? CgaPlane1 : CgaPlane0);
    if (GFX_COLOR_WHITE == color)
    {
        plane[VID_CGA_HIMONO_XY(left, top)] |= leftCorner;
        plane[VID_CGA_HIMONO_XY(right, top)] |= rightCorner;
    }
    else
    {
        plane[VID_CGA_HIMONO_XY(left, top)] &= ~leftCorner;
        plane[VID_CGA_HIMONO_XY(right, top)] &= ~rightCorner;
    }

    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[top / 2 * VID_CGA_HIMONO_LINE + byte] = horizontalFill;
    }

    // Bottom line
    plane = (far uint8_t *)((bottom % 2) ? CgaPlane1 : CgaPlane0);
    if (GFX_COLOR_WHITE == color)
    {
        plane[VID_CGA_HIMONO_XY(left, bottom)] |= leftCorner;
        plane[VID_CGA_HIMONO_XY(right, bottom)] |= rightCorner;
    }
    else
    {
        plane[VID_CGA_HIMONO_XY(left, bottom)] &= ~leftCorner;
        plane[VID_CGA_HIMONO_XY(right, bottom)] &= ~rightCorner;
    }

    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[bottom / 2 * VID_CGA_HIMONO_LINE + byte] = horizontalFill;
    }

    // Vertical lines
    plane = (far uint8_t *)((y % 2) ? CgaPlane1 : CgaPlane0);
    if (GFX_COLOR_WHITE == color)
    {
        for (int16_t line = y; line < bottom; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(left, line)] |= leftBorder;
            plane[VID_CGA_HIMONO_XY(right, line)] |= rightBorder;
        }
    }
    else
    {
        for (int16_t line = y; line < bottom; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(left, line)] &= ~leftBorder;
            plane[VID_CGA_HIMONO_XY(right, line)] &= ~rightBorder;
        }
    }

    plane = (far uint8_t *)((y % 2) ? CgaPlane0 : CgaPlane1);
    if (GFX_COLOR_WHITE == color)
    {
        for (int16_t line = y + 1; line < bottom; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(left, line)] |= leftBorder;
            plane[VID_CGA_HIMONO_XY(right, line)] |= rightBorder;
        }
    }
    else
    {
        for (int16_t line = y + 1; line < bottom; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(left, line)] &= ~leftBorder;
            plane[VID_CGA_HIMONO_XY(right, line)] &= ~rightBorder;
        }
    }

    return 0;
}

int
VidFillRectangle(GFX_DIMENSIONS *rect, uint16_t x, uint16_t y, GFX_COLOR color)
{
    uint16_t xe = x + rect->Width;
    uint16_t ye = y + rect->Height;

    far char *plane;
    uint8_t   leftStripe = (1 << (8 - (x % 8))) - 1;
    uint8_t   rightStripe = ~((1 << (8 - (xe % 8))) - 1);
    uint8_t   horizontalFill = (GFX_COLOR_WHITE == color) ? 0xFF : 0x00;

    // Vertical stripes
    plane = (far uint8_t *)((y % 2) ? CgaPlane1 : CgaPlane0);
    if (GFX_COLOR_WHITE == color)
    {
        for (int16_t line = y; line < ye; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(x, line)] |= leftStripe;
            plane[VID_CGA_HIMONO_XY(xe, line)] |= rightStripe;
        }
    }
    else
    {
        for (int16_t line = y; line < ye; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(x, line)] &= ~leftStripe;
            plane[VID_CGA_HIMONO_XY(xe, line)] &= ~rightStripe;
        }
    }

    plane = (far uint8_t *)((y % 2) ? CgaPlane0 : CgaPlane1);
    if (GFX_COLOR_WHITE == color)
    {
        for (int16_t line = y + 1; line < ye; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(x, line)] |= leftStripe;
            plane[VID_CGA_HIMONO_XY(xe, line)] |= rightStripe;
        }
    }
    else
    {
        for (int16_t line = y + 1; line < ye; line += 2)
        {
            plane[VID_CGA_HIMONO_XY(x, line)] &= ~leftStripe;
            plane[VID_CGA_HIMONO_XY(xe, line)] &= ~rightStripe;
        }
    }

    // Internal fill
    plane = (far uint8_t *)((y % 2) ? CgaPlane1 : CgaPlane0);
    for (int16_t line = y; line < ye; line += 2)
    {
        uint16_t lineOffset = line / 2 * VID_CGA_HIMONO_LINE;
        for (uint16_t byte = x / 8 + (0 != (x % 8)); byte < xe / 8; byte++)
        {
            plane[lineOffset + byte] = horizontalFill;
        }
    }

    plane = (far uint8_t *)((y % 2) ? CgaPlane0 : CgaPlane1);
    for (int16_t line = y + 1; line < ye; line += 2)
    {
        uint16_t lineOffset = line / 2 * VID_CGA_HIMONO_LINE;
        for (uint16_t byte = x / 8 + (0 != (x % 8)); byte < xe / 8; byte++)
        {
            plane[lineOffset + byte] = horizontalFill;
        }
    }

    return 0;
}

int
VidDrawText(const char *str, uint16_t x, uint16_t y)
{
    while (*str)
    {
        BiosVideoSetCursorPosition(0, (y << 8) | x++);
        BiosVideoWriteCharacter(0, *str, 0x80, 1);
        str++;
    }
}

void
VidLoadFont(void)
{
    _disable();
    PreviousFontPtr = _dos_getvect(INT_CGA_EXTENDED_FONT_PTR);
    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (isr)__VidExtendedFont);
    _enable();

    far const char *bfont =
        MK_FP(CGA_BASIC_FONT_SEGMENT, CGA_BASIC_FONT_OFFSET);
    char *                    xfont = __VidExtendedFont;
    VID_CHARACTER_DESCRIPTOR *fdata = __VidFontData;
    bool                      isDosBox = KerIsDosBox();

    if (isDosBox)
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

        if (NULL != fdata->Transformation)
        {
            FontExecuteGlyphTransformation(fdata->Transformation, xfont);
        }

        unsigned ovheight = fdata->Overlay[0] & 0xF;
        unsigned ovtop = (unsigned)fdata->Overlay[0] >> 4;

        if (isDosBox && (8 <= (ovtop + ovheight)))
        {
            ovheight = 7 - ovtop;
        }

        for (unsigned i = 0; i < ovheight; i++)
        {
            xfont[ovtop + i] |= fdata->Overlay[1 + i];
        }

        fdata++;
        xfont += 8;
    }
}

void
VidUnloadFont(void)
{
    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (isr)PreviousFontPtr);
}

char
VidConvertToLocal(uint16_t wc)
{
    if (0x80 > wc)
    {
        return wc;
    }

    uint8_t                   local = 0x80;
    VID_CHARACTER_DESCRIPTOR *fdata = __VidFontData;

    while (wc > fdata->CodePoint)
    {
        if (NULL != fdata->Overlay)
        {
            local++;
        }
        fdata++;
    }

    if (wc != fdata->CodePoint)
    {
        return '?';
    }

    if (NULL == fdata->Overlay)
    {
        return fdata->Base;
    }

    return local;
}

void
FontExecuteGlyphTransformation(const char *gxf, char *glyph)
{
    char *   selStart = glyph;
    unsigned selLength = 1;
    while (*gxf)
    {
        unsigned command = (unsigned)*gxf >> 4;
        unsigned param = (unsigned)*gxf & 0xF;

        switch (command)
        {
        case VID_GXF_CMD_GROW:
            selLength += param;
            break;

        case VID_GXF_CMD_SELECT:
            selStart = glyph + param;
            selLength = 1;
            break;

        case VID_GXF_CMD_MOVE:
            memmove(selStart + param, selStart, selLength);
            memset(selStart, 0, param);
            break;

        case VID_GXF_CMD_CLEAR:
            glyph[param] = 0;
            break;

        default:
            break;
        }

        gxf++;
    }
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
