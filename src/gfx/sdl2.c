#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <gfx.h>
#include <pal.h>
#include <platform/sdl2arch.h>

SDL_Window *_window = NULL;

static SDL_Renderer *_renderer = NULL;
static TTF_Font     *_font = NULL;
static int           _font_w, _font_h;
static int           _screen_w, _screen_h;
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
    TTF_SizeText(_font, "WW", &_font_w, &_font_h);
    _font_w /= 2;
    _font_h = TTF_FontHeight(_font);

    _screen_w = 80 * _font_w;
    _screen_h = 25 * _font_h;
    _window = SDL_CreateWindow(pal_get_version_string(), SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED, _screen_w, _screen_h,
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

    dim->width = _screen_w;
    dim->height = _screen_h;

    LOG("exit, %dx%d", dim->width, dim->height);
}

void
gfx_get_glyph_dimensions(gfx_dimensions *dim)
{
    dim->width = _font_w;
    dim->height = _font_h;
}

uint16_t
gfx_get_pixel_aspect(void)
{
    LOG("entry");

    uint16_t ratio = 64 * (1);

    LOG("exit, ratio: 1:%.2f", (float)ratio / 64.0f);
    return ratio;
}

unsigned
gfx_get_color_depth(void)
{
    return 24;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    LOG("entry, bm: %dx%d %ubpp (%u planes, %u octets per scanline), x: %u,"
        " y: %u",
        bm->width, bm->height, bm->bpp, bm->planes, bm->opl, x, y);

    if (1 != bm->planes)
    {
        errno = EFTYPE;
        return false;
    }

    if ((1 != bm->bpp) && (4 != bm->bpp) && (32 != bm->bpp))
    {
        errno = EFTYPE;
        return false;
    }

    SDL_Surface *screen = SDL_GetWindowSurface(_window);

    SDL_Rect src_rect = {0, 0, bm->width, abs(bm->height)};
    SDL_Rect dst_rect = {x, y, bm->width, abs(bm->height)};
    uint32_t format = SDL_PIXELFORMAT_XRGB8888;

    SDL_Surface *surface = NULL;

    if (1 == bm->bpp)
    {
        char *bits = (char *)bm->bits;
        char *end = bits + bm->opl * bm->height;
        while (bits < end)
        {
            *bits = ~*bits;
            bits++;
        }

        format = SDL_PIXELFORMAT_INDEX1MSB;
    }

    if (4 == bm->bpp)
    {
        surface = SDL_CreateRGBSurfaceWithFormat(0, bm->width, abs(bm->height),
                                                 32, SDL_PIXELFORMAT_XRGB8888);
        SDL_LockSurface(surface);

        uint8_t  *src = (uint8_t *)bm->bits;
        uint32_t *dst = (uint32_t *)surface->pixels;
        for (int y = 0; y < surface->h; y++)
        {
            for (int x = 0; x < surface->w; x += 2)
            {
                const SDL_Color *rgb = COLORS + (src[x / 2] >> 4);
                dst[x] = (rgb->r << 16) | (rgb->g << 8) | (rgb->b << 0);

                rgb = COLORS + (src[x / 2] & 0xF);
                dst[x + 1] = (rgb->r << 16) | (rgb->g << 8) | (rgb->b << 0);
            }

            src += bm->opl;
            dst += surface->w;
        }

        SDL_UnlockSurface(surface);
    }
    else
    {
        surface = SDL_CreateRGBSurfaceWithFormatFrom(
            bm->bits, bm->width, abs(bm->height), bm->bpp, bm->opl, format);
    }

    if (0 > bm->height)
    {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(_renderer, surface);
        SDL_RenderCopyEx(_renderer, texture, &src_rect, &dst_rect, 0, NULL,
                         SDL_FLIP_VERTICAL);
        SDL_DestroyTexture(texture);
    }
    else
    {
        SDL_BlitSurface(surface, &src_rect, screen, &dst_rect);
    }
    SDL_FreeSurface(surface);

    sdl2arch_present(_renderer);
    return true;
}

bool
gfx_draw_line(gfx_rect *rect, gfx_color color)
{
    LOG("entry, rect: %dx%d@%u,%u, color: %d", rect->width, rect->height,
        rect->left, rect->top, color);
    _rendering_text = false;

    _set_color(color);
    SDL_RenderDrawLine(_renderer, rect->left, rect->top,
                       rect->left + rect->width - ((1 == rect->height) ? 0 : 1),
                       rect->top + rect->height - ((1 == rect->width) ? 0 : 1));
    sdl2arch_present(_renderer);
    return true;
}

bool
gfx_draw_rectangle(gfx_rect *rect, gfx_color color)
{
    LOG("entry, rect: %dx%d@%u,%u, color: %d", rect->width, rect->height,
        rect->left, rect->top, color);
    _rendering_text = false;

    SDL_Rect sdl_rect = {rect->left - 1, rect->top - 1, rect->width + 2,
                         rect->height + 2};
    _set_color(color);
    SDL_RenderDrawRect(_renderer, &sdl_rect);
    sdl2arch_present(_renderer);
    return true;
}

bool
gfx_fill_rectangle(gfx_rect *rect, gfx_color color)
{
    LOG("entry, rect: %dx%d@%u,%u, color: %d", rect->width, rect->height,
        rect->left, rect->top, color);
    _rendering_text = false;

    SDL_Rect sdl_rect = {rect->left, rect->top, rect->width, rect->height};
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
    SDL_Rect rect = {x * _font_w, y * _font_h, surface->w, surface->h};

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

void
sdl2arch_set_window_title(const char *title)
{
    SDL_SetWindowTitle(_window, title);
}
