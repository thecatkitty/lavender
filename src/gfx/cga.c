#include <dos.h>
#include <graph.h>
#include <libi86/string.h>

#include <api/bios.h>
#include <drv.h>
#include <gfx.h>
#include <platform/dospc.h>

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

#define _xy(x, y) ((x) / 8 + (y / 2) * CGA_HIMONO_LINE)
#define _for_lines(start, end, body)                                           \
    {                                                                          \
        far char *plane = CGA_PLANE(start);                                    \
        for (int16_t line = (start); line < (end); line += 2)                  \
            body;                                                              \
                                                                               \
        plane = CGA_PLANE(start + 1);                                          \
        for (int16_t line = (start) + 1; line < (end); line += 2)              \
            body;                                                              \
    }

extern char            __gfx_xfont[];
static far char *const _plane0 = MK_FP(CGA_HIMONO_MEM, 0);
static far char *const _plane1 = MK_FP(CGA_HIMONO_MEM, CGA_HIMONO_PLANE);

#define CGA_PLANE(y) (((y) % 2) ? _plane1 : _plane0)

// Grayscale pattern brushes
enum
{
    BRUSH_BLACK,
    BRUSH_GRAY25,
    BRUSH_GRAY50,
    BRUSH_GRAY75,
    BRUSH_WHITE,
    BRUSH_MAX
};

#define BRUSH_HEIGHT 2

static const unsigned char BRUSHES[BRUSH_MAX][BRUSH_HEIGHT] = {
    [BRUSH_BLACK] = {0x00, 0x00},  // ........ ........
    [BRUSH_GRAY25] = {0x88, 0x22}, // #...#... ..#...#.
    [BRUSH_GRAY50] = {0xAA, 0x55}, // #.#.#.#. .#.#.#.#
    [BRUSH_GRAY75] = {0x77, 0xDD}, // .###.### ##.###.#
    [BRUSH_WHITE] = {0xFF, 0xFF}   // ######## ########
};

static uint16_t  _prev_mode;
static dospc_isr _prev_fontptr;

static void
_execute_glyph_trasformation(uint8_t idx, char *glyph)
{
    char    *sel_start = glyph;
    unsigned sel_length = 1;

    far const char *gxf = (far const char *)(__gfx_transformations + idx);
    while (*gxf)
    {
        unsigned command = (unsigned)*gxf >> 4;
        unsigned param = (unsigned)*gxf & 0xF;

        switch (command)
        {
        case GXF_CMD_GROW:
            sel_length += param;
            break;

        case GXF_CMD_SELECT:
            sel_start = glyph + param;
            sel_length = 1;
            break;

        case GXF_CMD_MOVE:
            memmove(sel_start + param, sel_start, sel_length);
            memset(sel_start, 0, param);
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

bool ddcall
cga_open(device *dev)
{
    // Set video mode
    _prev_mode = _getvideomode();
    if (!_setvideomode(_HRESBW))
    {
        errno = ENODEV;
        return false;
    }

    // Save and replace extended font pointer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    _disable();
    _prev_fontptr = _dos_getvect(INT_CGA_EXTENDED_FONT_PTR);
    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (dospc_isr)__gfx_xfont);
    _enable();
#pragma GCC diagnostic pop

    // Load font
    far const char *bfont =
        MK_FP(CGA_BASIC_FONT_SEGMENT, CGA_BASIC_FONT_OFFSET);
    char            *xfont = __gfx_xfont;
    const gfx_glyph *fdata = __gfx_font_8x8;
    bool             is_dosbox = dospc_is_dosbox();

    if (is_dosbox)
    {
        xfont++; // DOSBox ROM font is moved one line to the bottom
    }

    while (0xFFFF != fdata->codepoint)
    {
        if (0 == fdata->overlay)
        {
            fdata++;
            continue;
        }

        if (0 != fdata->base)
        {
            _fmemcpy(xfont, bfont + 8 * fdata->base, 8);
        }

        if (0 != fdata->transformation)
        {
            _execute_glyph_trasformation(fdata->transformation, xfont);
        }

        far const char *overlay =
            (far const char *)(__gfx_overlays + fdata->overlay);
        unsigned ovheight = overlay[0] & 0xF;
        unsigned ovtop = (unsigned)overlay[0] >> 4;

        if (is_dosbox && (8 <= (ovtop + ovheight)))
        {
            ovheight = 7 - ovtop;
        }

        for (unsigned i = 0; i < ovheight; i++)
        {
            xfont[ovtop + i] |= overlay[1 + i];
        }

        fdata++;
        xfont += 8;
    }

    return true;
}

bool ddcall
cga_get_property(device *dev, gfx_property property, void *out)
{
    switch (property)
    {
    case GFX_PROPERTY_SCREEN_SIZE: {
        gfx_dimensions *dim = (gfx_dimensions *)out;
        dim->width = CGA_HIMONO_WIDTH;
        dim->height = CGA_HIMONO_HEIGHT;
        return true;
    }

    case GFX_PROPERTY_GLYPH_SIZE: {
        gfx_dimensions *dim = (gfx_dimensions *)out;
        dim->width = CGA_CHARACTER_HEIGHT;
        dim->height = CGA_CHARACTER_HEIGHT;
        return true;
    }

    default:
        return false;
    }
}

bool ddcall
cga_draw_bitmap(device *dev, gfx_bitmap *bm, int x, int y)
{
    if ((1 != bm->planes) || (1 != bm->bpp))
    {
        errno = EFTYPE;
        return false;
    }

    x >>= 3;

    far char *bits = bm->bits;
    far char *plane0 = _plane0;
    far char *plane1 = _plane1;

    uint16_t offset = (y / 2) * CGA_HIMONO_LINE + x;
    plane0 += offset;
    plane1 += offset;

    for (uint16_t line = 0; line < bm->height; line += 2)
    {
        _fmemcpy(plane0, bits, bm->opl);
        plane0 += CGA_HIMONO_LINE;
        bits += bm->opl;

        _fmemcpy(plane1, bits, bm->opl);
        plane1 += CGA_HIMONO_LINE;
        bits += bm->opl;
    }

    return true;
}

static const uint8_t *
_get_brush(gfx_color color)
{
    switch (color)
    {
    case GFX_COLOR_BLACK:
    case GFX_COLOR_NAVY:
    case GFX_COLOR_GREEN:
    case GFX_COLOR_MAROON:
        return BRUSHES[BRUSH_BLACK];
    case GFX_COLOR_TEAL:
    case GFX_COLOR_PURPLE:
    case GFX_COLOR_OLIVE:
    case GFX_COLOR_BLUE:
    case GFX_COLOR_LIME:
    case GFX_COLOR_RED:
        return BRUSHES[BRUSH_GRAY25];
    case GFX_COLOR_GRAY:
    case GFX_COLOR_CYAN:
    case GFX_COLOR_FUCHSIA:
    case GFX_COLOR_YELLOW:
        return BRUSHES[BRUSH_GRAY50];
    case GFX_COLOR_SILVER:
        return BRUSHES[BRUSH_GRAY75];
    case GFX_COLOR_WHITE:
        return BRUSHES[BRUSH_WHITE];
    default:
        return BRUSHES[BRUSH_GRAY50];
    }
}

static void
_draw_block(far char *plane,
            uint16_t  x,
            uint16_t  y,
            char      block,
            char      mask,
            char      pattern)
{
    far char *dst = plane + _xy(x, y);
    *dst = (*dst & mask) | (pattern & block);
}

static void
_draw_line(
    far char *plane, uint16_t y, uint16_t left, uint16_t right, char pattern)
{
    uint16_t offset = y / 2 * CGA_HIMONO_LINE;
    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[offset + byte] = pattern;
    }
}

bool ddcall
cga_draw_line(device *dev, gfx_rect *rect, gfx_color color)
{
    uint16_t left = rect->left;
    uint16_t y = rect->top;
    uint16_t right = rect->left + rect->width;
    uint16_t bottom = y + rect->height;

    const uint8_t *brush = _get_brush(color);
    char           pattern = brush[y % BRUSH_HEIGHT];

    if (1 == rect->height)
    {
        far void *plane = CGA_PLANE(y);
        uint8_t   lmask = (1 << (8 - (left % 8))) - 1;
        uint8_t   rmask = ~((1 << (7 - (right % 8))) - 1);

        _draw_block(plane, left, y, lmask, ~lmask, pattern);
        _draw_line(plane, y, left, right, pattern);
        _draw_block(plane, right, y, rmask, ~rmask, pattern);

        return true;
    }

    if (1 == rect->width)
    {
        uint8_t mask = 1 << (7 - (left % 8));

        _for_lines(y, bottom, {
            pattern = brush[line % BRUSH_HEIGHT];
            _draw_block(plane, left, line, mask, ~mask, pattern);
        });

        return true;
    }

    errno = EINVAL;
    return false;
}

bool ddcall
cga_draw_rectangle(device *dev, gfx_rect *rect, gfx_color color)
{
    uint16_t x = rect->left;
    uint16_t y = rect->top;

    uint16_t left = x - 1;
    uint16_t right = x + rect->width;
    uint16_t top = y - 1;
    uint16_t bottom = y + rect->height;

    far char *plane;
    uint8_t   lmask = (1 << (8 - (left % 8))) - 1;
    uint8_t   rmask = ~((1 << (7 - (right % 8))) - 1);
    uint8_t   lborder = 1 << (7 - (left % 8));
    uint8_t   rborder = 1 << (7 - (right % 8));

    const uint8_t *brush = _get_brush(color);
    char           pattern;

    // Top line
    plane = CGA_PLANE(top);
    pattern = brush[top % BRUSH_HEIGHT];
    _draw_block(plane, left, top, lmask, ~lmask, pattern);
    _draw_block(plane, right, top, rmask, ~rmask, pattern);

    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[top / 2 * CGA_HIMONO_LINE + byte] = pattern;
    }

    // Bottom line
    plane = CGA_PLANE(bottom);
    pattern = brush[bottom % BRUSH_HEIGHT];
    _draw_block(plane, left, bottom, lmask, ~lmask, pattern);
    _draw_block(plane, right, bottom, rmask, ~rmask, pattern);

    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        plane[bottom / 2 * CGA_HIMONO_LINE + byte] = pattern;
    }

    // Vertical lines
    _for_lines(y, bottom, {
        pattern = brush[line % BRUSH_HEIGHT];
        _draw_block(plane, left, line, lborder, ~lborder, pattern);
        _draw_block(plane, right, line, rborder, ~rborder, pattern);
    });

    return true;
}

bool ddcall
cga_fill_rectangle(device *dev, gfx_rect *rect, gfx_color color)
{
    uint16_t x = rect->left;
    uint16_t y = rect->top;

    uint16_t left = x;
    uint16_t right = x + rect->width;
    uint16_t bottom = y + rect->height;

    uint8_t lmask = (1 << (8 - (x % 8))) - 1;
    uint8_t rmask = ~((1 << (8 - (right % 8))) - 1);

    const uint8_t *brush = _get_brush(color);
    char           pattern;

    // Vertical stripes
    _for_lines(y, bottom, {
        pattern = brush[line % BRUSH_HEIGHT];
        _draw_block(plane, left, line, lmask, ~lmask, pattern);
        _draw_block(plane, right, line, rmask, ~rmask, pattern);
    });

    // Internal fill
    _for_lines(
        y, bottom,
        _draw_line(plane, line, left, right, brush[line % BRUSH_HEIGHT]));

    return true;
}

bool ddcall
cga_draw_text(device *dev, const char *str, uint16_t x, uint16_t y)
{
    while (*str)
    {
        bios_set_cursor_position(0, (y << 8) | x++);
        bios_write_character(0, *str, 0x80, 1);
        str++;
    }

    return true;
}

void ddcall
cga_close(device *dev)
{
    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (dospc_isr)_prev_fontptr);
    _setvideomode(_prev_mode);
}

static device DRV_DATA         _dev = {"cga", "Color Graphics Adapter"};
static gfx_device_ops DRV_DATA _ops = {
    .open = cga_open,
    .close = cga_close,
    .get_property = cga_get_property,
    .draw_line = cga_draw_line,
    .draw_rectangle = cga_draw_rectangle,
    .fill_rectangle = cga_fill_rectangle,
    .draw_bitmap = cga_draw_bitmap,
    .draw_text = cga_draw_text,
};

DRV_INIT(cga)(void)
{
    _dev.ops = &_ops;
    return gfx_register_device(&_dev);
}
