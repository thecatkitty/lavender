#ifdef CONFIG_ANDREA
#include <andrea.h>
#endif
#include <arch/dos.h>
#include <arch/dos/winoldap.h>
#include <gfx.h>
#include <pal.h>

#define MAX_DEVICES 1

extern int ddcall
__cga_init(void);

static device  _devices[MAX_DEVICES] = {{{0}}};
static device *_dev = NULL;
#if defined(CONFIG_ANDREA)
static uint16_t _driver = 0;
#endif

int ddcall
gfx_register_device(far device *dev)
{
    for (int i = 0; i < MAX_DEVICES; i++)
    {
        if (NULL == _devices[i].ops)
        {
            _fmemcpy(_devices + i, dev, sizeof(*dev));
            _dev = _devices + i;
            return 0;
        }
    }

    errno = ENOMEM;
    return -errno;
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(gfx_register_device);
#endif

#if defined(CONFIG_ANDREA)
static bool
_try_driver(const char *name, void *data)
{
    uint16_t driver = dos_load_driver(name);
    if (0 == driver)
    {
        return true;
    }

    if (NULL != _dev)
    {
        _driver = driver;
        return false;
    }

    dos_unload_driver(driver);
    return true;
}
#endif // CONFIG_ANDREA

bool
gfx_initialize(void)
{
    memset(_devices, 0, sizeof(_devices));
#if defined(CONFIG_ANDREA)
    pal_enum_assets(_try_driver, "gfx*.sys", NULL);
    if (0 == _driver)
#endif // CONFIG_ANDREA
    {
        __cga_init();
    }

    if (NULL == _dev)
    {
        return false;
    }

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
gfx_get_font_data(gfx_glyph_data *data)
{
    return gfx_device_get_property(_dev, GFX_PROPERTY_GLYPH_DATA, data);
}

unsigned
gfx_get_color_depth(void)
{
    unsigned depth = 1;
    gfx_device_get_property(_dev, GFX_PROPERTY_COLOR_DEPTH, &depth);
    return depth;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    return gfx_device_draw_bitmap(_dev, bm, x, y);
}

bool
gfx_draw_line(const gfx_rect *rect, gfx_color color)
{
    return gfx_device_draw_line(_dev, rect, color);
}

bool
gfx_draw_rectangle(const gfx_rect *rect, gfx_color color)
{
    return gfx_device_draw_rectangle(_dev, rect, color);
}

bool
gfx_fill_rectangle(const gfx_rect *rect, gfx_color color)
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

bool
gfx_set_title(const char *title)
{
    if (!dos_is_windows())
    {
        return false;
    }

    char buffer[80];
    utf8_encode(title, buffer, pal_wctoa);
    return 1 == winoldap_set_title(buffer);
}
