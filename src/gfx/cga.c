#include <dos.h>
#include <graph.h>
#include <libi86/string.h>

#include <api/bios.h>
#include <api/dos.h>
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

#define CGA_PLANE0   ((far char *)MK_FP(CGA_HIMONO_MEM, 0))
#define CGA_PLANE1   ((far char *)MK_FP(CGA_HIMONO_MEM, CGA_HIMONO_PLANE))
#define CGA_PLANE(y) (((y) % 2) ? CGA_PLANE1 : CGA_PLANE0)

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

static const uint8_t DRV_RDAT BRUSHES[BRUSH_MAX][BRUSH_HEIGHT] = {
    [BRUSH_BLACK] = {0x00, 0x00},  // ........ ........
    [BRUSH_GRAY25] = {0x88, 0x22}, // #...#... ..#...#.
    [BRUSH_GRAY50] = {0xAA, 0x55}, // #.#.#.#. .#.#.#.#
    [BRUSH_GRAY75] = {0x77, 0xDD}, // .###.### ##.###.#
    [BRUSH_WHITE] = {0xFF, 0xFF}   // ######## ########
};

static const unsigned DRV_RDAT MAPPINGS[] = {
    [GFX_COLOR_BLACK] = BRUSH_BLACK,   [GFX_COLOR_NAVY] = BRUSH_BLACK,
    [GFX_COLOR_GREEN] = BRUSH_BLACK,   [GFX_COLOR_TEAL] = BRUSH_GRAY25,
    [GFX_COLOR_MAROON] = BRUSH_BLACK,  [GFX_COLOR_PURPLE] = BRUSH_GRAY25,
    [GFX_COLOR_OLIVE] = BRUSH_GRAY25,  [GFX_COLOR_SILVER] = BRUSH_GRAY75,
    [GFX_COLOR_GRAY] = BRUSH_GRAY50,   [GFX_COLOR_BLUE] = BRUSH_GRAY25,
    [GFX_COLOR_LIME] = BRUSH_GRAY25,   [GFX_COLOR_CYAN] = BRUSH_GRAY50,
    [GFX_COLOR_RED] = BRUSH_GRAY25,    [GFX_COLOR_FUCHSIA] = BRUSH_GRAY50,
    [GFX_COLOR_YELLOW] = BRUSH_GRAY50, [GFX_COLOR_WHITE] = BRUSH_WHITE,
};

typedef struct
{
    far void *font;
    uint16_t  old_mode;
    dospc_isr old_font;
} cga_data;

#define get_data(dev) ((far cga_data *)((dev)->data))

static void
_execute_glyph_trasformation(uint8_t idx, far char *glyph)
{
    far char *sel_start = glyph;
    unsigned  sel_length = 1;

    far const char *gxf = (far const char *)(__gfx_xforms + idx);
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
            _fmemmove(sel_start + param, sel_start, sel_length);
            _fmemset(sel_start, 0, param);
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
    if (NULL == (dev->data = _fmalloc(sizeof(cga_data))))
    {
        return false;
    }

    far cga_data *data = get_data(dev);
    if (NULL == (data->font = _fmalloc(128 * CGA_CHARACTER_HEIGHT)))
    {
        _ffree(dev->data);
        return false;
    }

    // Set video mode
    data->old_mode = bios_get_video_mode();
    bios_set_video_mode(_HRESBW);

    // Save and replace extended font pointer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    _disable();
    data->old_font = _dos_getvect(INT_CGA_EXTENDED_FONT_PTR);
    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (dospc_isr)data->font);
    _enable();
#pragma GCC diagnostic pop

    // Load font
    far const char *bfont =
        MK_FP(CGA_BASIC_FONT_SEGMENT, CGA_BASIC_FONT_OFFSET);
    far char            *xfont = (far char *)data->font;
    far const gfx_glyph *fdata = __gfx_font_8x8;
    bool                 is_dosbox = dospc_is_dosbox();

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

    case GFX_PROPERTY_GLYPH_DATA: {
        gfx_glyph_data *data = (gfx_glyph_data *)out;
        *data = __gfx_font_8x8;
        return true;
    }

    case GFX_PROPERTY_COLOR_DEPTH: {
        unsigned *depth = (unsigned *)out;
        *depth = 1;
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
    far char *plane0 = CGA_PLANE0;
    far char *plane1 = CGA_PLANE1;

    uint16_t offset = (y / 2) * CGA_HIMONO_LINE + x;
    plane0 += offset;
    plane1 += offset;

    int line_span = CGA_HIMONO_LINE;
    int lines = bm->height;
    int width = (bm->width + 7) / 8;
    if (0 > bm->height)
    {
        plane0 -= (bm->height + 1) / 2 * CGA_HIMONO_LINE;
        plane1 -= (bm->height + 1) / 2 * CGA_HIMONO_LINE;
        line_span = -CGA_HIMONO_LINE;
        lines = -bm->height;
    }

    for (uint16_t line = 0; line < lines; line += 2)
    {
        if (0 < line_span)
        {
            _fmemcpy(plane0, bits, width);
            plane0 += line_span;
            bits += bm->opl;
        }

        _fmemcpy(plane1, bits, width);
        plane1 += line_span;
        bits += bm->opl;

        if (0 > line_span)
        {
            _fmemcpy(plane0, bits, width);
            plane0 += line_span;
            bits += bm->opl;
        }
    }

    return true;
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
cga_draw_line(device *dev, const gfx_rect *rect, gfx_color color)
{
    uint16_t left = rect->left;
    uint16_t y = rect->top;
    uint16_t right = rect->left + rect->width;
    uint16_t bottom = y + rect->height;

    far const uint8_t *brush = BRUSHES[MAPPINGS[color]];
    char               pattern = brush[y % BRUSH_HEIGHT];

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
cga_draw_rectangle(device *dev, const gfx_rect *rect, gfx_color color)
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

    far const uint8_t *brush = BRUSHES[MAPPINGS[color]];
    char               pattern;

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
cga_fill_rectangle(device *dev, const gfx_rect *rect, gfx_color color)
{
    uint16_t x = rect->left;
    uint16_t y = rect->top;

    uint16_t left = x;
    uint16_t right = x + rect->width;
    uint16_t bottom = y + rect->height;

    uint8_t lmask = (1 << (8 - (x % 8))) - 1;
    uint8_t rmask = ~((1 << (8 - (right % 8))) - 1);

    far const uint8_t *brush = BRUSHES[MAPPINGS[color]];
    char               pattern;

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

static void
_load_cell(char *buffer, uint16_t x, uint16_t y)
{
    size_t offset = y * 8 / 2 * CGA_HIMONO_LINE + x;
    for (int i = 0; i < 8; i += 2)
    {
        buffer[i] = CGA_PLANE0[offset];
        buffer[i + 1] = CGA_PLANE1[offset];
        offset += CGA_HIMONO_LINE;
    }
}

static void
_combine_cell(const char *buffer, uint16_t x, uint16_t y)
{
    size_t offset = y * 8 / 2 * CGA_HIMONO_LINE + x;
    for (int i = 0; i < 8; i += 2)
    {
        CGA_PLANE0[offset] ^= buffer[i];
        CGA_PLANE1[offset] ^= buffer[i + 1];
        offset += CGA_HIMONO_LINE;
    }
}

bool ddcall
cga_draw_text(device *dev, const char *str, uint16_t x, uint16_t y)
{
    while (*str)
    {
        bios_set_cursor_position(0, (y << 8) | x);
        if (((y == GFX_LINES - 1) &&      // use BIOS to prevent scrolling
             (x == GFX_COLUMNS - 1)) ||   // on the last cell
            strchr("\a\b\e\n\r\t", *str)) // and for special character
        {
            bios_write_character(0, *str, 0x80, 1);
        }
        else
        {
            char cell[8];
            _load_cell(cell, x, y);
            dos_putc(*str);
            _combine_cell(cell, x, y);
        }

        str++;
        x++;
    }

    return true;
}

void ddcall
cga_close(device *dev)
{
    far cga_data *data = get_data(dev);

    _dos_setvect(INT_CGA_EXTENDED_FONT_PTR, (dospc_isr)data->old_font);
    bios_set_video_mode(data->old_mode);
    _ffree(data->font);
    _ffree(dev->data);
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

#ifdef LOADABLE
int ddcall
drv_deinit(void)
{
    return 0;
}

ANDREA_EXPORT(drv_deinit);
#endif
