#ifndef GFX_DDI_H
#define GFX_DDI_H

#include <ddi.h>
#include <gfx/types.h>

typedef enum
{
    GFX_PROPERTY_SCREEN_SIZE = 1,
    GFX_PROPERTY_GLYPH_SIZE = 2,
    GFX_PROPERTY_GLYPH_DATA = 3,
    GFX_PROPERTY_COLOR_DEPTH = 4,
} gfx_property;

typedef struct
{
    bool ddcall (*open)(device *dev);
    void ddcall (*close)(device *dev);
    bool ddcall (*get_property)(device *dev, gfx_property property, void *out);

    bool ddcall (*draw_line)(device         *dev,
                             const gfx_rect *rect,
                             gfx_color       color);
    bool ddcall (*draw_rectangle)(device         *dev,
                                  const gfx_rect *rect,
                                  gfx_color       color);
    bool ddcall (*fill_rectangle)(device         *dev,
                                  const gfx_rect *rect,
                                  gfx_color       color);

    bool ddcall (*draw_bitmap)(device *dev, gfx_bitmap *bm, int x, int y);
    bool ddcall (*draw_text)(device     *dev,
                             const char *str,
                             uint16_t    x,
                             uint16_t    y);
} gfx_device_ops;

#define GFX_DEVICE_OPS(dev)   ((far gfx_device_ops *)((dev)->ops))
#define gfx_device_open(dev)  GFX_DEVICE_OPS(dev)->open((dev))
#define gfx_device_close(dev) GFX_DEVICE_OPS(dev)->close((dev))
#define gfx_device_get_property(dev, property, out)                            \
    GFX_DEVICE_OPS(dev)->get_property((dev), (property), (out))

#define gfx_device_draw_line(dev, rect, color)                                 \
    GFX_DEVICE_OPS(dev)->draw_line((dev), (rect), (color))
#define gfx_device_draw_rectangle(dev, rect, color)                            \
    GFX_DEVICE_OPS(dev)->draw_rectangle((dev), (rect), (color))
#define gfx_device_fill_rectangle(dev, rect, color)                            \
    GFX_DEVICE_OPS(dev)->fill_rectangle((dev), (rect), (color))

#define gfx_device_draw_bitmap(dev, bm, x, y)                                  \
    GFX_DEVICE_OPS(dev)->draw_bitmap((dev), (bm), (x), (y))
#define gfx_device_draw_text(dev, str, x, y)                                   \
    GFX_DEVICE_OPS(dev)->draw_text((dev), (str), (x), (y))

#if defined(CONFIG_HAVE_GFX_DEVICES)
extern int ddcall
gfx_register_device(far device *dev);
#endif

#endif // GFX_DDI_H
