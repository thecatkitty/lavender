#include <conio.h>
#include <graph.h>

#include <api/bios.h>
#include <drv.h>
#include <gfx.h>

#define EGA_HIRES_WIDTH      640
#define EGA_HIRES_HEIGHT     350
#define EGA_HIRES_LINE       (EGA_HIRES_WIDTH / 8)
#define EGA_HIRES_MEM        0xA000 // Video memory base
#define EGA_CHARACTER_WIDTH  8
#define EGA_CHARACTER_HEIGHT 14
#define EGA_PLANES           4

typedef struct
{
    uint16_t     old_mode;
    far uint8_t *font;
} ega_data;

#define get_data(dev) ((far ega_data *)((dev)->data))

bool ddcall
ega_open(device *dev)
{
    if (NULL == (dev->data = _fmalloc(sizeof(ega_data))))
    {
        return false;
    }

    far ega_data *data = get_data(dev);

    // Set video mode
    data->old_mode = bios_get_video_mode();
    bios_set_video_mode(_ERESCOLOR);
    data->font = bios_get_font_information(2); // 8x14
    return true;
}

bool ddcall
ega_get_property(device *dev, gfx_property property, void *out)
{
    switch (property)
    {
    case GFX_PROPERTY_SCREEN_SIZE: {
        gfx_dimensions *dim = (gfx_dimensions *)out;
        dim->width = EGA_HIRES_WIDTH;
        dim->height = EGA_HIRES_HEIGHT;
        return true;
    }

    case GFX_PROPERTY_GLYPH_SIZE: {
        gfx_dimensions *dim = (gfx_dimensions *)out;
        dim->width = EGA_CHARACTER_WIDTH;
        dim->height = EGA_CHARACTER_HEIGHT;
        return true;
    }

    default:
        return false;
    }
}

static inline void
_read_plane(unsigned i)
{
    outpw(0x03CE, (i << 8) | 0x04);
}

static inline void
_write_planes(unsigned i)
{
    outpw(0x03C4, (i << 8) | 0x02);
}

static inline void
_set_plane(unsigned i)
{
    _read_plane(i);
    _write_planes(1 << i);
}

bool ddcall
ega_draw_bitmap(device *dev, gfx_bitmap *bm, int x, int y)
{
    return true;
}

static void
_draw_hline(uint16_t top,
            uint16_t left,
            uint16_t width,
            uint8_t  pattern,
            uint8_t  lmask,
            uint8_t  rmask)
{
    far uint8_t *fb = MK_FP(EGA_HIRES_MEM, top * EGA_HIRES_LINE);

    uint16_t right = left + width;
    for (uint16_t byte = left / 8 + (0 != (left % 8)); byte < right / 8; byte++)
    {
        fb[byte] = pattern;
    }

    fb[left / 8] = (pattern & lmask) | (fb[left / 8] & ~lmask);
    fb[right / 8] = (pattern & rmask) | (fb[right / 8] & ~rmask);
}

bool ddcall
ega_draw_line(device *dev, gfx_rect *rect, gfx_color color)
{
    uint8_t pattern;

    if (1 == rect->height)
    {
        uint16_t right = rect->left + rect->width;
        uint8_t  lmask = (1 << (8 - (rect->left % 8))) - 1;
        uint8_t  rmask = ~((1 << (7 - (right % 8))) - 1);

        for (int i = 0; i < EGA_PLANES; i++)
        {
            _set_plane(i);
            pattern = (color & (1 << i)) ? 0xFF : 0x00;
            _draw_hline(rect->top, rect->left, rect->width, pattern, lmask,
                        rmask);
        }
        return true;
    }

    if (1 == rect->width)
    {
        uint8_t mask = 1 << (7 - (rect->left % 8));

        for (int i = 0; i < EGA_PLANES; i++)
        {
            _set_plane(i);
            pattern = (color & (1 << i)) ? 0xFF : 0x00;

            far uint8_t *fb = MK_FP(EGA_HIRES_MEM, rect->top * EGA_HIRES_LINE);
            for (uint16_t line = 0; line < rect->height; line++)
            {
                fb[rect->left / 8] =
                    (pattern & mask) | (fb[rect->left / 8] & ~mask);
                fb += EGA_HIRES_LINE;
            }
        }

        return true;
    }

    return false;
}

bool ddcall
ega_draw_rectangle(device *dev, gfx_rect *rect, gfx_color color)
{
    uint16_t left = rect->left - 1;
    uint16_t right = rect->left + rect->width;
    uint16_t top = rect->top - 1;
    uint16_t bottom = rect->top + rect->height;

    uint8_t lmask = (1 << (8 - (left % 8))) - 1;
    uint8_t rmask = ~((1 << (7 - (right % 8))) - 1);
    uint8_t lborder = 1 << (7 - (left % 8));
    uint8_t rborder = 1 << (7 - (right % 8));

    for (int i = 0; i < EGA_PLANES; i++)
    {
        _set_plane(i);
        uint8_t pattern = (color & (1 << i)) ? 0xFF : 0x00;

        // Vertical lines
        far uint8_t *fb = MK_FP(EGA_HIRES_MEM, (top + 1) * EGA_HIRES_LINE);
        for (uint16_t line = top + 1; line < bottom; line++)
        {
            fb[left / 8] = (pattern & lborder) | (fb[left / 8] & ~lborder);
            fb[right / 8] = (pattern & rborder) | (fb[right / 8] & ~rborder);
            fb += EGA_HIRES_LINE;
        }

        // Horizontal line
        _draw_hline(top, left, rect->width + 1, pattern, lmask, rmask);
        _draw_hline(bottom, left, rect->width + 1, pattern, lmask, rmask);
    }

    return true;
}

bool ddcall
ega_fill_rectangle(device *dev, gfx_rect *rect, gfx_color color)
{
    uint16_t left = rect->left;
    uint16_t right = left + rect->width;
    uint16_t top = rect->top;
    uint16_t bottom = top + rect->height;
    uint16_t rbyte = rect->width / 8;
    unsigned lspan = 0 != (left % 8);

    uint8_t lmask = (1 << (8 - (left % 8))) - 1;
    uint8_t rmask = ~((1 << (8 - (right % 8))) - 1);

    // Light up active channels in inner area
    far uint8_t *fb = MK_FP(EGA_HIRES_MEM, top * EGA_HIRES_LINE + (left / 8));
    _write_planes(color);
    for (uint16_t line = top; line < bottom; line++)
    {
        _fmemset(fb + lspan, 0xFF, rbyte - lspan);
        fb += EGA_HIRES_LINE;
    }

    // Put out inactive channels in inner area
    fb = MK_FP(EGA_HIRES_MEM, top * EGA_HIRES_LINE + (left / 8));
    _write_planes(~color & 0xF);
    for (uint16_t line = top; line < bottom; line++)
    {
        _fmemset(fb + lspan, 0x00, rbyte - lspan);
        fb += EGA_HIRES_LINE;
    }

    // Draw vertical border area
    for (int i = 0; i < EGA_PLANES; i++)
    {
        _set_plane(i);
        uint8_t pattern = (color & (1 << i)) ? 0xFF : 0x00;

        fb = MK_FP(EGA_HIRES_MEM, top * EGA_HIRES_LINE + (left / 8));
        for (uint16_t line = top; line < bottom; line++)
        {
            fb[0] = (pattern & lmask) | (fb[0] & ~lmask);
            fb[rbyte] = (pattern & rmask) | (fb[rbyte] & ~rmask);
            fb += EGA_HIRES_LINE;
        }
    }

    return true;
}

bool ddcall
ega_draw_text(device *dev, const char *str, uint16_t x, uint16_t y)
{
    far ega_data *data = get_data(dev);
    far uint8_t  *fb =
        MK_FP(EGA_HIRES_MEM, y * (EGA_HIRES_LINE * EGA_CHARACTER_HEIGHT) + x);

    for (int i = 0; i < EGA_PLANES; i++)
    {
        _set_plane(i);

        for (int i = 0; str[i]; i++)
        {
            far uint8_t *glyph = data->font + (str[i] * EGA_CHARACTER_HEIGHT);
            for (int line = 0; line < EGA_CHARACTER_HEIGHT; line++)
            {
                fb[line * EGA_HIRES_LINE + i] ^= glyph[line];
            }
        }
    }

    return true;
}

void ddcall
ega_close(device *dev)
{
    far ega_data *data = get_data(dev);

    bios_set_video_mode(data->old_mode);
    _ffree(dev->data);
}

static device DRV_DATA         _dev = {"ega", "Enhanced Graphics Adapter"};
static gfx_device_ops DRV_DATA _ops = {
    .open = ega_open,
    .close = ega_close,
    .get_property = ega_get_property,
    .draw_line = ega_draw_line,
    .draw_rectangle = ega_draw_rectangle,
    .fill_rectangle = ega_fill_rectangle,
    .draw_bitmap = ega_draw_bitmap,
    .draw_text = ega_draw_text,
};

DRV_INIT(ega)(void)
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
