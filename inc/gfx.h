#ifndef _GFX_H_
#define _GFX_H_

#include <dev.h>

typedef struct
{
    int16_t  width;
    int16_t  height;
    uint16_t opl;
    uint8_t  bpp;
    void    *bits;
} gfx_bitmap;

typedef struct
{
    int width;
    int height;
} gfx_dimensions;

typedef struct
{
    int left;
    int top;
    int width;
    int height;
} gfx_rect;

typedef enum
{
    GFX_COLOR_BLACK = 0,
    GFX_COLOR_NAVY = 1,
    GFX_COLOR_GREEN = 2,
    GFX_COLOR_TEAL = 3,
    GFX_COLOR_MAROON = 4,
    GFX_COLOR_PURPLE = 5,
    GFX_COLOR_OLIVE = 6,
    GFX_COLOR_SILVER = 7,
    GFX_COLOR_GRAY = 8,
    GFX_COLOR_BLUE = 9,
    GFX_COLOR_LIME = 10,
    GFX_COLOR_CYAN = 11,
    GFX_COLOR_RED = 12,
    GFX_COLOR_FUCHSIA = 13,
    GFX_COLOR_YELLOW = 14,
    GFX_COLOR_WHITE = 15,
    GFX_COLOR_UNKNOWN = -1
} gfx_color;

typedef enum
{
    GFX_PROPERTY_SCREEN_SIZE = 1,
    GFX_PROPERTY_GLYPH_SIZE = 2,
    GFX_PROPERTY_GLYPH_DATA = 3,
    GFX_PROPERTY_COLOR_DEPTH = 4,
} gfx_property;

#define GFX_COLUMNS 80
#define GFX_LINES   25

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

#define gfx_device_open(dev) (((far gfx_device_ops *)((dev)->ops))->open((dev)))
#define gfx_device_close(dev)                                                  \
    (((far gfx_device_ops *)((dev)->ops))->close((dev)))
#define gfx_device_get_property(dev, property, out)                            \
    (((far gfx_device_ops *)((dev)->ops))                                      \
         ->get_property((dev), (property), (out)))

#define gfx_device_draw_line(dev, rect, color)                                 \
    (((far gfx_device_ops *)((dev)->ops))->draw_line((dev), (rect), (color)))
#define gfx_device_draw_rectangle(dev, rect, color)                            \
    (((far gfx_device_ops *)((dev)->ops))                                      \
         ->draw_rectangle((dev), (rect), (color)))
#define gfx_device_fill_rectangle(dev, rect, color)                            \
    (((far gfx_device_ops *)((dev)->ops))                                      \
         ->fill_rectangle((dev), (rect), (color)))

#define gfx_device_draw_bitmap(dev, bm, x, y)                                  \
    (((far gfx_device_ops *)((dev)->ops))->draw_bitmap((dev), (bm), (x), (y)))
#define gfx_device_draw_text(dev, str, x, y)                                   \
    (((far gfx_device_ops *)((dev)->ops))->draw_text((dev), (str), (x), (y)))

#pragma pack(push, 1)
typedef struct
{
    uint16_t codepoint;
    char     base;
    uint8_t  overlay;
    uint8_t  transformation;
} gfx_glyph;
#pragma pack(pop)

typedef far const gfx_glyph *gfx_glyph_data;

#if defined(__ia16__)
extern int ddcall
gfx_register_device(far device *dev);
#endif

extern bool
gfx_initialize(void);

extern void
gfx_cleanup(void);

// Get width and height of the screen area in pixels
extern void
gfx_get_screen_dimensions(gfx_dimensions *dim);

// Get width and height of a glyph in pixels
extern void
gfx_get_glyph_dimensions(gfx_dimensions *dim);

// Get pixel aspect ratio
// Returns pixel aspect ratio (PAR = 64 / value), default value on error
extern uint16_t
gfx_get_pixel_aspect(void);

#if !defined(__ia16__)
#define GFX_HAS_SCALE
// Get display scaling factor
extern float
gfx_get_scale(void);
#endif

#if defined(__ia16__)
extern bool
gfx_get_font_data(gfx_glyph_data *data);
#endif

extern unsigned
gfx_get_color_depth(void);

extern bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y);

extern bool
gfx_draw_line(const gfx_rect *rect, gfx_color color);

extern bool
gfx_draw_rectangle(const gfx_rect *rect, gfx_color color);

extern bool
gfx_fill_rectangle(const gfx_rect *rect, gfx_color color);

extern bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y);

extern gfx_color
gfx_get_color(const char *str);

extern bool
gfx_set_title(const char *title);

#endif // _GFX_H_
