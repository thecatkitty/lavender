#include <stdio.h>

#if defined(_WIN32)

#include <arch/windows.h>

#else

#include <sharizard/drawing.h>
#include <sharizard/host.h>
#include <sharizard/input.h>

#include <gfx.h>

#include "../resource.h"

#define SHARIZARD_USES_CANVAS

#endif

#include "enc_impl.h"

static char _brand[100];

bool
encui_enter(shiz_page *pages, unsigned count)
{
    shiz_wizard wizard = {pages, count, _brand};
#if defined(_WIN32)
    wizard.owner = (uintptr_t)windows_get_hwnd();
#endif

    if (0 == _brand[0])
    {
        snprintf(_brand, sizeof(_brand),
                 "%s\n"
                 "https://celones.pl/lavender\n"
                 "(C) 2021-2026 Mateusz Karcz",
                 pal_get_version_string());
    }

    return (0 == shiz_enter(&wizard));
}

#if defined(SHARIZARD_USES_CANVAS)

static uint32_t _ticks_per_sec = 0;

static void
_dimensions_to_vec2i(_In_ const gfx_dimensions *dims, _Out_ shiz_vec2i *vec)
{
    vec->x = dims->width;
    vec->y = dims->height;
}

static gfx_color
_map_color(shiz_color color)
{
    return (gfx_color)color;
}

shizerr
shizd_get_cell_size(_In_opt_ void *ctx, _Out_ shiz_vec2i *cell)
{
    gfx_dimensions dims;
    gfx_get_glyph_dimensions(&dims);
    _dimensions_to_vec2i(&dims, cell);
    return 0;
}

shizerr
shizd_get_viewbox_size(_In_opt_ void *ctx, _Out_ shiz_vec2i *viewbox)
{
    gfx_dimensions dims;
    gfx_get_screen_dimensions(&dims);
    _dimensions_to_vec2i(&dims, viewbox);
    return 0;
}

shizerr
shizd_draw_bitmap(_In_opt_ void *ctx, int x, int y, _In_ const shiz_bitmap *bm)
{
    gfx_bitmap bitmap = {0};
    bitmap.width = bm->size.x;
    bitmap.height = bm->size.y;
    bitmap.opl = bm->stride;
    bitmap.bpp = (bm->format == SHIZ_PXFORMAT_MONO1)      ? 1
                 : (bm->format == SHIZ_PXFORMAT_IRGB1111) ? 4
                                                          : 24;
    bitmap.bits = (void *)bm->pixels;
    return gfx_draw_bitmap(&bitmap, x, y) ? 0 : -EIO;
}

shizerr
shizd_draw_line(_In_opt_ void         *ctx,
                int                    x,
                int                    y,
                _In_ const shiz_vec2i *extent,
                shiz_color             color)
{
    gfx_rect rect = {x, y, extent->x, extent->y};
    return gfx_draw_line(&rect, _map_color(color)) ? 0 : -EIO;
}

shizerr
shizd_draw_rectangle(_In_opt_ void         *ctx,
                     int                    x,
                     int                    y,
                     _In_ const shiz_vec2i *extent,
                     shiz_color             color)
{
    gfx_rect rect = {x, y, extent->x, extent->y};
    return gfx_draw_rectangle(&rect, _map_color(color)) ? 0 : -EIO;
}

shizerr
shizd_fill_rectangle(_In_opt_ void         *ctx,
                     int                    x,
                     int                    y,
                     _In_ const shiz_vec2i *extent,
                     shiz_color             color)
{
    gfx_rect rect = {x, y, extent->x, extent->y};
    return gfx_fill_rectangle(&rect, _map_color(color)) ? 0 : -EIO;
}

shizerr
shizd_draw_text(_In_opt_ void     *ctx,
                unsigned           x,
                unsigned           y,
                _In_z_ const char *str)
{
    return gfx_draw_text(str, x, y) ? 0 : -EIO;
}

shizerr
shizd_lock_surface(_In_opt_ void *ctx)
{
    pal_disable_mouse();
    return 0;
}

shizerr
shizd_unlock_surface(_In_opt_ void *ctx, int lock)
{
    pal_enable_mouse();
    return 0;
}

uint16_t
shizi_get_key(_In_opt_ void *ctx)
{
    uint16_t key = pal_get_keystroke();
    switch (key)
    {
    case 0:
        return 0;
    case VK_BACK:
        return SHIZK_BACKSPACE;
    case VK_RETURN:
        return SHIZK_RETURN;
    case VK_ESCAPE:
        return SHIZK_ESCAPE;
    case VK_PRIOR:
        return SHIZK_PAGEUP;
    case VK_LEFT:
        return SHIZK_LEFT;
    case VK_RIGHT:
        return SHIZK_RIGHT;
    case VK_DELETE:
        return SHIZK_DELETE;
    case VK_F1:
        return SHIZK_F1;
    case VK_F2:
        return SHIZK_F2;
    case VK_F3:
        return SHIZK_F3;
    case VK_F4:
        return SHIZK_F4;
    case VK_F5:
        return SHIZK_F5;
    case VK_F6:
        return SHIZK_F6;
    case VK_F7:
        return SHIZK_F7;
    case VK_F8:
        return SHIZK_F8;
    case VK_OEM_MINUS:
        return '-';
    default:
        if ((('0' <= key) && ('9' >= key)) || (('A' <= key) && ('Z' >= key)))
        {
            return key;
        }
    }

    return 0;
}

uint16_t
shizi_get_mouse(_In_opt_ void *ctx, _Out_ int *x, _Out_ int *y)
{
    uint16_t mx, my, buttons = pal_get_mouse(&mx, &my);
    *x = mx;
    *y = my;
    return buttons;
}

shiz_ms
shizh_get_clock(_In_opt_ void *ctx)
{
    uint32_t ticks = pal_get_counter();
    if (0 == _ticks_per_sec)
    {
        _ticks_per_sec = pal_get_ticks(1000);
    }
    return ticks * 1000 / _ticks_per_sec;
}

shizerr
shizh_load_string(_In_opt_ void                   *ctx,
                  int                              id,
                  _Out_writes_opt_z_(buffsz) char *buff,
                  size_t                           buffsz)
{
    switch (id)
    {
    case SHIZSID_CANCEL:
        id = IDS_CANCEL;
        break;
    case SHIZSID_BACK:
        id = IDS_BACK;
        break;
    case SHIZSID_NEXT:
        id = IDS_NEXT;
        break;
    }

    return pal_load_string(id, buff, buffsz - 1);
}

#endif
