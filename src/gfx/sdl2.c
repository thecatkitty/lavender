#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <gfx.h>
#include <pal.h>
#include <platform/sdl2arch.h>

SDL_Window *_window = NULL;

static SDL_Renderer *_renderer = NULL;
static TTF_Font     *_font = NULL;
static int           _font_w, _font_h;
static bool          _rendering_text = false;

static const SDL_Color COLORS[] = {
    [GFX_COLOR_BLACK] = {0, 0, 0},      [GFX_COLOR_NAVY] = {0, 0, 128},
    [GFX_COLOR_GREEN] = {0, 128, 0},    [GFX_COLOR_TEAL] = {0, 128, 128},
    [GFX_COLOR_MAROON] = {128, 0, 0},   [GFX_COLOR_PURPLE] = {128, 0, 128},
    [GFX_COLOR_OLIVE] = {128, 128, 0},  [GFX_COLOR_SILVER] = {192, 192, 192},
    [GFX_COLOR_GRAY] = {128, 128, 128}, [GFX_COLOR_BLUE] = {0, 0, 255},
    [GFX_COLOR_LIME] = {0, 255, 0},     [GFX_COLOR_CYAN] = {0, 255, 255},
    [GFX_COLOR_RED] = {255, 0, 0},      [GFX_COLOR_FUCHSIA] = {255, 0, 255},
    [GFX_COLOR_YELLOW] = {255, 255, 0}, [GFX_COLOR_WHITE] = {255, 255, 255}};

extern void
sdl2arch_present(SDL_Renderer *renderer);

static void
_set_color(gfx_color color)
{
    const SDL_Color *rgb = COLORS + color;
    SDL_SetRenderDrawColor(_renderer, rgb->r, rgb->g, rgb->b, 0);
}

static void
_blend_subtractive(SDL_Surface *surface, SDL_Rect *rect)
{
    SDL_LockSurface(surface);
    Uint8 *src = (Uint8 *)surface->pixels;

    SDL_Surface *screen = SDL_GetWindowSurface(_window);
    SDL_LockSurface(screen);
    Uint8 *dst = (Uint8 *)screen->pixels;

    int maxx = SDL_min(surface->w, screen->w - rect->x);
    int maxy = SDL_min(surface->h, screen->h - rect->y);
    for (int cy = 0; cy < maxy; cy++)
    {
        for (int cx = 0; cx < maxx; cx++)
        {
            Uint8 *srcpx = src + cy * surface->pitch + cx * 4;
            Uint8 *dstpx =
                dst + (rect->y + cy) * screen->pitch + (rect->x + cx) * 4;
            srcpx[0] = SDL_max((int)srcpx[0] - dstpx[0], 0);
            srcpx[1] = SDL_max((int)srcpx[1] - dstpx[1], 0);
            srcpx[2] = SDL_max((int)srcpx[2] - dstpx[2], 0);
        }
    }

    SDL_UnlockSurface(screen);
    SDL_UnlockSurface(surface);
}

bool
gfx_initialize(void)
{
    LOG("entry");

    if (0 > SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        LOG("cannot initialize SDL video. %s", SDL_GetError());
        return false;
    }

    if (0 > TTF_Init())
    {
        LOG("cannot initialize SDL_ttf. %s", SDL_GetError());
        return false;
    }

    const char *font_path = sdl2arch_get_font();
    if (NULL == font_path)
    {
        LOG("cannot determine font");
        return false;
    }

    _font = TTF_OpenFont(font_path, 12);
    if (NULL == _font)
    {
        LOG("cannot open font '%s'. %s", font_path, SDL_GetError());
        return false;
    }

    LOG("font: '%s'", font_path);
    TTF_SizeText(_font, "W", &_font_w, &_font_h);

    _window = SDL_CreateWindow(pal_get_version_string(), SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, 80 * _font_w, 25 * 16,
                               SDL_WINDOW_SHOWN);
    if (NULL == _window)
    {
        LOG("cannot create window. %s", SDL_GetError());
        return false;
    }

    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE);
    if (NULL == _renderer)
    {
        LOG("cannot create renderer. %s", SDL_GetError());
        return false;
    }

    SDL_RenderClear(_renderer);
    sdl2arch_present(_renderer);
    return true;
}

void
gfx_cleanup(void)
{
    LOG("entry");

    if (_renderer)
    {
        SDL_DestroyRenderer(_renderer);
    }

    if (_window)
    {
        SDL_DestroyWindow(_window);
    }

    if (_font)
    {
        TTF_CloseFont(_font);
        TTF_Quit();
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void
gfx_get_screen_dimensions(gfx_dimensions *dim)
{
    LOG("entry");

    dim->width = 80 * _font_w;
    dim->height = 200;

    LOG("exit, %dx%d", dim->width, dim->height);
}

void
gfx_get_glyph_dimensions(gfx_dimensions *dim)
{
    dim->width = _font_w;
    dim->height = 8;
}

uint16_t
gfx_get_pixel_aspect(void)
{
    LOG("entry");

    uint16_t ratio = 64 * (2);

    LOG("exit, ratio: 1:%.2f", (float)ratio / 64.0f);
    return ratio;
}

static void
_draw_bitmap_1bpp(gfx_bitmap *bm,
                  uint16_t    x,
                  uint16_t    y,
                  unsigned    screen_pitch,
                  uint32_t   *pixels)
{
    for (int cy = 0; cy < bm->height; cy++)
    {
        int base = (y + cy) * (screen_pitch / sizeof(uint32_t) * 2) + x;
        for (int cx = 0; cx < bm->width; cx++)
        {
            uint8_t  byte = ((const uint8_t *)bm->bits)[cy * bm->opl + cx / 8];
            uint32_t value = (byte & (0x80 >> (cx % 8))) ? 0xFFFFFF : 0;
            pixels[base + cx] = value;
            pixels[base + (screen_pitch / sizeof(uint32_t)) + cx] = value;
        }
    }
}

static void
_draw_bitmap_32bpp(gfx_bitmap *bm,
                   uint16_t    x,
                   uint16_t    y,
                   unsigned    screen_pitch,
                   uint32_t   *pixels)
{
    int maxy = (bm->height < 0) ? -bm->height : bm->height;
    int pitch = ((bm->height < 0) ? -1 : 1) * (bm->opl / sizeof(uint32_t));
    const uint32_t *bits = (const uint32_t *)bm->bits;
    if (pitch < 0)
    {
        bits += pitch * (bm->height + 1);
    }

    for (int cy = 0; cy < maxy; cy++)
    {
        int base = (y + cy) * (screen_pitch / sizeof(uint32_t) * 2) + x;
        for (int cx = 0; cx < bm->width; cx++)
        {
            uint32_t value = bits[cy * pitch + cx];
            pixels[base + cx] = value;
            pixels[base + (screen_pitch / sizeof(uint32_t)) + cx] = value;
        }
    }
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, uint16_t x, uint16_t y)
{
    LOG("entry, bm: %dx%d %ubpp (%u planes, %u octets per scanline), x: %u,"
        " y: %u",
        bm->width, bm->height, bm->bpp, bm->planes, bm->opl, x, y);

    if (1 != bm->planes)
    {
        errno = EFTYPE;
        return false;
    }

    if ((1 != bm->bpp) && (32 != bm->bpp))
    {
        errno = EFTYPE;
        return false;
    }

    SDL_Surface *screen = SDL_GetWindowSurface(_window);
    SDL_LockSurface(screen);
    uint32_t *pixels = (uint32_t *)screen->pixels;

    if (1 == bm->bpp)
    {
        _draw_bitmap_1bpp(bm, x, y, screen->pitch, pixels);
    }
    else
    {
        _draw_bitmap_32bpp(bm, x, y, screen->pitch, pixels);
    }

    SDL_UnlockSurface(screen);

    SDL_Rect rect = {x, y * 2, bm->width, bm->height * 2};
    SDL_UpdateWindowSurfaceRects(_window, &rect, 1);
    return true;
}

bool
gfx_draw_line(gfx_dimensions *dim, uint16_t x, uint16_t y, gfx_color color)
{
    LOG("entry, dim: %dx%d, x: %u, y: %u, color: %d", dim->width, dim->height,
        x, y, color);
    _rendering_text = false;

    _set_color(color);
    SDL_RenderDrawLine(_renderer, x, y * 2, x + dim->width,
                       (y + dim->height - 1) * 2);
    SDL_RenderDrawLine(_renderer, x, y * 2 + 1, x + dim->width,
                       (y + dim->height - 1) * 2 + 1);
    sdl2arch_present(_renderer);
    return true;
}

bool
gfx_draw_rectangle(gfx_dimensions *rect,
                   uint16_t        x,
                   uint16_t        y,
                   gfx_color       color)
{
    LOG("entry, dim: %dx%d, x: %u, y: %u, color: %d", rect->width, rect->height,
        x, y, color);
    _rendering_text = false;

    SDL_Rect sdl_rect = {x - 1, (y - 1) * 2, rect->width + 2,
                         (rect->height + 2) * 2};
    _set_color(color);
    SDL_RenderDrawRect(_renderer, &sdl_rect);
    sdl_rect.y += 1;
    sdl_rect.h -= 2;
    SDL_RenderDrawRect(_renderer, &sdl_rect);
    sdl2arch_present(_renderer);
    return true;
}

bool
gfx_fill_rectangle(gfx_dimensions *rect,
                   uint16_t        x,
                   uint16_t        y,
                   gfx_color       color)
{
    LOG("entry, dim: %dx%d, x: %u, y: %u, color: %d", rect->width, rect->height,
        x, y, color);
    _rendering_text = false;

    SDL_Rect sdl_rect = {x, y * 2, rect->width, rect->height * 2};
    _set_color(color);
    SDL_RenderFillRect(_renderer, &sdl_rect);
    sdl2arch_present(_renderer);
    return true;
}

bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y)
{
    LOG("entry, str: '%s', x: %u, y: %u", str, x, y);

    if (0 == strlen(str))
    {
        return true;
    }

    SDL_Surface *surface =
        TTF_RenderUTF8_Blended(_font, str, COLORS[GFX_COLOR_WHITE]);
    SDL_Rect rect = {x * _font_w, y * 16, surface->w, surface->h};

    if (!_rendering_text)
    {
        SDL_RenderPresent(_renderer);
    }
    _rendering_text = true;
    _blend_subtractive(surface, &rect);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(_renderer, surface);
    SDL_RenderCopy(_renderer, texture, NULL, &rect);
    sdl2arch_present(_renderer);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return true;
}
