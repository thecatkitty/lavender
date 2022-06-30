#include <dos.h>
#include <libi86/string.h>

#include <api/bios.h>
#include <pal/dospc.h>
#include <vid.h>

#include "glyph.h"

#define CGA_HIMONO_WIDTH     640
#define CGA_HIMONO_HEIGHT    200
#define CGA_HIMONO_LINE      (CGA_HIMONO_WIDTH / 8)
#define CGA_HIMONO_MEM       0xB800 // Video memory base
#define CGA_HIMONO_PLANE     0x2000 // Odd plane offset
#define CGA_CHARACTER_HEIGHT 8      // Text mode character height

// IVT pointer to font data over code point 127
#define INT_CGA_EXTENDED_FONT_PTR 31

// 0FFA6Eh - ROM font base
#define CGA_BASIC_FONT_SEGMENT 0xFFA0
#define CGA_BASIC_FONT_OFFSET  0x6E

#define VID_CGA_HIMONO_XY(x, y) ((x) / 8 + (y / 2) * CGA_HIMONO_LINE)
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

extern char            __vid_xfont[];
extern const vid_glyph __vid_font_8x8[];

static far void *const CGA_PLANE0 = MK_FP(CGA_HIMONO_MEM, 0);
static far void *const CGA_PLANE1 = MK_FP(CGA_HIMONO_MEM, CGA_HIMONO_PLANE);

#define CGA_PLANE(y) (((y) % 2) ? CGA_PLANE1 : CGA_PLANE0)

static const char BRUSH_BLACK[] = {0x00};
static const char BRUSH_WHITE[] = {0xFF};
static const char BRUSH_GRAY50[] = {0x55, 0xAA};

static uint16_t  s_PreviousMode;
static dospc_isr s_PreviousFontPtr;

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
FontLoad(void);

static void
FontExecuteGlyphTransformation(const char *gxf, char *glyph);

static int
_read_edid(edid_block *edid);

bool
VidInitialize(void)
{
    FontLoad();

    s_PreviousMode = bios_get_video_mode() & 0xFF;
    bios_set_video_mode(BIOS_VIDEO_MODE_CGAHIMONO);
    return true;
}

void
VidGetScreenDimensions(GFX_DIMENSIONS *dim)
{
    dim->Width = CGA_HIMONO_WIDTH;
    dim->Height = CGA_HIMONO_HEIGHT;
}

int
VidDrawBitmap(GFX_BITMAP *bm, uint16_t x, uint16_t y)
{
    if ((1 != bm->Planes) || (1 != bm->BitsPerPixel))
    {
        ERR(VID_FORMAT);
    }

    far void *bits = bm->Bits;
    far void *plane0 = CGA_PLANE0;
    far void *plane1 = CGA_PLANE1;
    plane0 += x + y * (CGA_HIMONO_LINE / 2);
    plane1 += x + y * (CGA_HIMONO_LINE / 2);

    for (uint16_t line = 0; line < bm->Height; line += 2)
    {
        _fmemcpy(plane0, bits, bm->WidthBytes);
        plane0 += CGA_HIMONO_LINE;
        bits += bm->WidthBytes;

        _fmemcpy(plane1, bits, bm->WidthBytes);
        plane1 += CGA_HIMONO_LINE;
        bits += bm->WidthBytes;
    }
}

int
VidDrawLine(GFX_DIMENSIONS *dim, uint16_t x, uint16_t y, GFX_COLOR color)
{
    uint16_t left = x;
    uint16_t right = x + dim->Width;

    if (1 != dim->Height)
    {
        ERR(KER_UNSUPPORTED);
    }

    far void *plane = CGA_PLANE(y);
    uint8_t   leftMask = (1 << (8 - (left % 8))) - 1;
    uint8_t   rightMask = ~((1 << (7 - (right % 8))) - 1);

    int         brushHeight;
    const char *brush = CgaGetBrush(color, &brushHeight);
    char        pattern = brush[y % brushHeight];

    CgaDrawBlock(plane, left, y, leftMask, ~leftMask, pattern);
    CgaDrawLine(plane, y, left, right, pattern);
    CgaDrawBlock(plane, right, y, rightMask, ~rightMask, pattern);

    return 0;
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
        plane[top / 2 * CGA_HIMONO_LINE + byte] = pattern;
    }

    // Bottom line
    plane = CGA_PLANE(bottom);
    pattern = brush[bottom % brushHeight];
    CgaDrawBlock(plane, left, bottom, leftMask, ~leftMask, pattern);
    CgaDrawBlock(plane, right, bottom, rightMask, ~rightMask, pattern);

    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[bottom / 2 * CGA_HIMONO_LINE + byte] = pattern;
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
        bios_set_cursor_position(0, (y << 8) | x++);
        bios_write_character(0, *str, 0x80, 1);
        str++;
    }
}

void
FontLoad(void)
{
    _disable();
    s_PreviousFontPtr = _dos_getvect(INT_CGA_EXTENDED_FONT_PTR);
    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (dospc_isr)__vid_xfont);
    _enable();

    far const char *bfont =
        MK_FP(CGA_BASIC_FONT_SEGMENT, CGA_BASIC_FONT_OFFSET);
    char            *xfont = __vid_xfont;
    const vid_glyph *fdata = __vid_font_8x8;
    bool             isDosBox = dospc_is_dosbox();

    if (isDosBox)
    {
        xfont++; // DOSBox ROM font is moved one line to the bottom
    }

    while (0xFFFF != fdata->codepoint)
    {
        if (NULL == fdata->overlay)
        {
            fdata++;
            continue;
        }

        if (0 != fdata->base)
        {
            _fmemcpy(xfont, bfont + 8 * fdata->base, 8);
        }

        if (NULL != fdata->transformation)
        {
            FontExecuteGlyphTransformation(fdata->transformation, xfont);
        }

        unsigned ovheight = fdata->overlay[0] & 0xF;
        unsigned ovtop = (unsigned)fdata->overlay[0] >> 4;

        if (isDosBox && (8 <= (ovtop + ovheight)))
        {
            ovheight = 7 - ovtop;
        }

        for (unsigned i = 0; i < ovheight; i++)
        {
            xfont[ovtop + i] |= fdata->overlay[1 + i];
        }

        fdata++;
        xfont += 8;
    }
}

void
VidCleanup(void)
{
    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (dospc_isr)s_PreviousFontPtr);
    bios_set_video_mode((uint8_t)s_PreviousMode);
}

char
VidConvertToLocal(uint16_t wc)
{
    if (0x80 > wc)
    {
        return wc;
    }

    uint8_t          local = 0x80;
    const vid_glyph *fdata = __vid_font_8x8;

    while (wc > fdata->codepoint)
    {
        if (NULL != fdata->overlay)
        {
            local++;
        }
        fdata++;
    }

    if (wc != fdata->codepoint)
    {
        return '?';
    }

    if (NULL == fdata->overlay)
    {
        return fdata->base;
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
    uint16_t offset = y / 2 * CGA_HIMONO_LINE;
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
        *height = sizeof(BRUSH_BLACK);
        return BRUSH_BLACK;
    case GFX_COLOR_GRAY50:
        *height = sizeof(BRUSH_GRAY50);
        return BRUSH_GRAY50;
    default:
        *height = sizeof(BRUSH_WHITE);
        return BRUSH_WHITE;
    }
}

void
FontExecuteGlyphTransformation(const char *gxf, char *glyph)
{
    char    *selStart = glyph;
    unsigned selLength = 1;
    while (*gxf)
    {
        unsigned command = (unsigned)*gxf >> 4;
        unsigned param = (unsigned)*gxf & 0xF;

        switch (command)
        {
        case GXF_CMD_GROW:
            selLength += param;
            break;

        case GXF_CMD_SELECT:
            selStart = glyph + param;
            selLength = 1;
            break;

        case GXF_CMD_MOVE:
            memmove(selStart + param, selStart, selLength);
            memset(selStart, 0, param);
            break;

        case GXF_CMD_CLEAR:
            glyph[param] = 0;
            break;

        default:
            break;
        }

        gxf++;
    }
}
