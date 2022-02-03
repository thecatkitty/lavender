#include <libi86/string.h>

#include <api/bios.h>
#include <api/dos.h>
#include <dev/cga.h>
#include <ker.h>
#include <vid.h>

#define VID_PAR(dx, dy, sx, sy)                                                \
    (uint8_t)(64U * (unsigned)dy * (unsigned)sx / (unsigned)dx / (unsigned)sy)
#define VID_CGA_HIMONO_XY(x, y) ((x) / 8 + (y / 2) * VID_CGA_HIMONO_LINE)

#define CGA_PLANE(y) (((y) % 2) ? CgaPlane1 : CgaPlane0)
#define CGA_FOR_LINES(start, end, body)                                        \
    {                                                                          \
        far char *plane = CGA_PLANE(start);                                    \
        for (int16_t line = (start); line < (end); line += 2)                  \
            body;                                                              \
                                                                               \
        plane = CGA_PLANE(start + 1);                                          \
        for (int16_t line = (start) + 1; line < (end); line += 2)              \
            body;                                                              \
    }

extern char                     __VidExtendedFont[];
extern VID_CHARACTER_DESCRIPTOR __VidFontData[];

static isr       PreviousFontPtr;
static far void *CgaPlane0 = MK_FP(CGA_HIMONO_MEM, 0);
static far void *CgaPlane1 = MK_FP(CGA_HIMONO_MEM, CGA_HIMONO_PLANE);

static const char BrushBlack[] = {0x00};
static const char BrushWhite[] = {0xFF};
static const char BrushGray50[] = {0x55, 0xAA};

static void
CgaDrawBlock(far char *plane,
             uint16_t  x,
             uint16_t  y,
             char      block,
             char      mask,
             char      pattern);

static void
CgaDrawLine(
    far char *plane, uint16_t y, uint16_t left, uint16_t right, char pattern);

static const char *
CgaGetBrush(GFX_COLOR color, int *height);

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
    uint8_t   leftMask = (1 << (8 - (left % 8))) - 1;
    uint8_t   rightMask = ~((1 << (7 - (right % 8))) - 1);
    uint8_t   leftBorder = 1 << (7 - (left % 8));
    uint8_t   rightBorder = 1 << (7 - (right % 8));

    int         brushHeight;
    const char *brush = CgaGetBrush(color, &brushHeight);
    char        pattern;

    // Top line
    plane = CGA_PLANE(top);
    pattern = brush[top % brushHeight];
    CgaDrawBlock(plane, left, top, leftMask, ~leftMask, pattern);
    CgaDrawBlock(plane, right, top, rightMask, ~rightMask, pattern);

    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[top / 2 * VID_CGA_HIMONO_LINE + byte] = pattern;
    }

    // Bottom line
    plane = CGA_PLANE(bottom);
    pattern = brush[bottom % brushHeight];
    CgaDrawBlock(plane, left, bottom, leftMask, ~leftMask, pattern);
    CgaDrawBlock(plane, right, bottom, rightMask, ~rightMask, pattern);

    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[bottom / 2 * VID_CGA_HIMONO_LINE + byte] = pattern;
    }

    // Vertical lines
    CGA_FOR_LINES(y, bottom, {
        pattern = brush[line % brushHeight];
        CgaDrawBlock(plane, left, line, leftBorder, ~leftBorder, pattern);
        CgaDrawBlock(plane, right, line, rightBorder, ~rightBorder, pattern);
    });

    return 0;
}

int
VidFillRectangle(GFX_DIMENSIONS *rect, uint16_t x, uint16_t y, GFX_COLOR color)
{
    uint16_t left = x;
    uint16_t right = x + rect->Width;
    uint16_t top = y;
    uint16_t bottom = y + rect->Height;

    uint8_t leftMask = (1 << (8 - (x % 8))) - 1;
    uint8_t rightMask = ~((1 << (8 - (right % 8))) - 1);

    int         brushHeight;
    const char *brush = CgaGetBrush(color, &brushHeight);
    char        pattern;

    // Vertical stripes
    CGA_FOR_LINES(y, bottom, {
        pattern = brush[line % brushHeight];
        CgaDrawBlock(plane, left, line, leftMask, ~leftMask, pattern);
        CgaDrawBlock(plane, right, line, rightMask, ~rightMask, pattern);
    });

    // Internal fill
    CGA_FOR_LINES(
        y, bottom,
        CgaDrawLine(plane, line, left, right, brush[line % brushHeight]));

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
CgaDrawBlock(far char *plane,
             uint16_t  x,
             uint16_t  y,
             char      block,
             char      mask,
             char      pattern)
{
    far char *dst = plane + VID_CGA_HIMONO_XY(x, y);
    *dst = (*dst & mask) | (pattern & block);
}

void
CgaDrawLine(
    far char *plane, uint16_t y, uint16_t left, uint16_t right, char pattern)
{
    uint16_t offset = y / 2 * VID_CGA_HIMONO_LINE;
    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[offset + byte] = pattern;
    }
}

const char *
CgaGetBrush(GFX_COLOR color, int *height)
{
    switch (color)
    {
    case GFX_COLOR_BLACK:
        *height = sizeof(BrushBlack);
        return BrushBlack;
    case GFX_COLOR_GRAY50:
        *height = sizeof(BrushGray50);
        return BrushGray50;
    default:
        *height = sizeof(BrushWhite);
        return BrushWhite;
    }
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
