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

bool ddcall
ega_draw_bitmap(device *dev, gfx_bitmap *bm, int x, int y)
{
    return true;
}

bool ddcall
ega_draw_line(device *dev, gfx_rect *rect, gfx_color color)
{
    return true;
}

bool ddcall
ega_draw_rectangle(device *dev, gfx_rect *rect, gfx_color color)
{
    return true;
}

bool ddcall
ega_fill_rectangle(device *dev, gfx_rect *rect, gfx_color color)
{
    return true;
}

bool ddcall
ega_draw_text(device *dev, const char *str, uint16_t x, uint16_t y)
{
    far ega_data *data = get_data(dev);
    far uint8_t  *fb = MK_FP(EGA_HIRES_MEM, y * EGA_CHARACTER_HEIGHT + x);

    for (int i = 0; str[i]; i++)
    {
        far uint8_t *glyph = data->font + (*str * EGA_CHARACTER_HEIGHT);
        for (int line = 0; line < EGA_CHARACTER_HEIGHT; line++)
        {
            fb[line * EGA_HIRES_LINE + i] = glyph[line];
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
