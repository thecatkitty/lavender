#include <assert.h>
#include <gfx.h>
#include <platform/dospc.h>

#define MAX_DEVICES 1

extern int ddcall
__cga_init(void);

static device  _devices[MAX_DEVICES] = {{{0}}};
static device *_dev;

int ddcall
gfx_register_device(far device *dev)
{
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (NULL == _devices[i].ops)
        {
            _fmemcpy(_devices + i, dev, sizeof(*dev));
            return 0;
        }
    }

    errno = ENOMEM;
    return -errno;
}

bool
gfx_initialize(void)
{
    __cga_init();
    _dev = _devices + 0;
    return gfx_device_open(_dev);
}

void
gfx_get_screen_dimensions(gfx_dimensions *dim)
{
    gfx_device_get_property(_dev, GFX_PROPERTY_SCREEN_SIZE, dim);
}

void
gfx_get_glyph_dimensions(gfx_dimensions *dim)
{
    gfx_device_get_property(_dev, GFX_PROPERTY_GLYPH_SIZE, dim);
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    return gfx_device_draw_bitmap(_dev, bm, x, y);
}

bool
gfx_draw_line(gfx_rect *rect, gfx_color color)
{
    return gfx_device_draw_line(_dev, rect, color);
}

bool
gfx_draw_rectangle(gfx_rect *rect, gfx_color color)
{
    return gfx_device_draw_rectangle(_dev, rect, color);
}

bool
gfx_fill_rectangle(gfx_rect *rect, gfx_color color)
{
    return gfx_device_fill_rectangle(_dev, rect, color);
}

bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y)
{
    return gfx_device_draw_text(_dev, str, x, y);
}

void
gfx_cleanup(void)
{
    return gfx_device_close(_dev);
}
