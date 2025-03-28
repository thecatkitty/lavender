#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <arch/sdl2.h>
#include <gfx.h>
#include <pal.h>

SDL_Window *_window = NULL;

static SDL_Renderer *_renderer = NULL;
static TTF_Font     *_font = NULL;
static int           _font_w, _font_h;
static int           _screen_w = 0, _screen_h = 0;
static bool          _rendering_text = false;
static int           _scale;

static const SDL_Color COLORS[] = {
    [GFX_COLOR_BLACK] = {0, 0, 0},      [GFX_COLOR_NAVY] = {0, 0, 128},
    [GFX_COLOR_GREEN] = {0, 128, 0},    [GFX_COLOR_TEAL] = {0, 128, 128},
    [GFX_COLOR_MAROON] = {128, 0, 0},   [GFX_COLOR_PURPLE] = {128, 0, 128},
    [GFX_COLOR_OLIVE] = {128, 128, 0},  [GFX_COLOR_SILVER] = {192, 192, 192},
    [GFX_COLOR_GRAY] = {128, 128, 128}, [GFX_COLOR_BLUE] = {0, 0, 255},
    [GFX_COLOR_LIME] = {0, 255, 0},     [GFX_COLOR_CYAN] = {0, 255, 255},
    [GFX_COLOR_RED] = {255, 0, 0},      [GFX_COLOR_FUCHSIA] = {255, 0, 255},
    [GFX_COLOR_YELLOW] = {255, 255, 0}, [GFX_COLOR_WHITE] = {255, 255, 255}};

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
sdl2_set_scale(int scale)
{
    LOG("entry, scale: %d", scale);

    _scale = (scale < 1) ? 1 : scale;

    const char *font_path = sdl2_get_font();
    if (NULL == font_path)
    {
        LOG("cannot determine font");
        return false;
    }

    if (NULL != _font)
    {
        TTF_CloseFont(_font);
    }

    _font = TTF_OpenFont(font_path, _scale * 12);
    if (NULL == _font)
    {
        LOG("cannot open font '%s'. %s", font_path, SDL_GetError());
        return false;
    }

    LOG("font: '%s'", font_path);
    TTF_SizeText(_font, "WW", &_font_w, &_font_h);
    _font_w /= 2;
    _font_h = TTF_FontHeight(_font);

    SDL_Rect old = {0, 0}, new = {0, 0};

    old.w = _screen_w;
    old.h = _screen_h;
    _screen_w = GFX_COLUMNS * _font_w;
    _screen_h = GFX_LINES * _font_h;
    new.w = _screen_w;
    new.h = _screen_h;

    SDL_Surface *screen = SDL_GetWindowSurface(_window);
    SDL_Surface *saved = SDL_CreateRGBSurface(0, old.w, old.h, 32, 0, 0, 0, 0);
    SDL_BlitSurface(screen, &old, saved, &old);
    SDL_SetWindowSize(_window, _screen_w, _screen_h);

    screen = SDL_GetWindowSurface(_window);
    SDL_BlitScaled(saved, &old, screen, &new);
    SDL_FreeSurface(saved);
    SDL_UpdateWindowSurface(_window);
    return true;
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

    _window =
        SDL_CreateWindow(pal_get_version_string(), SDL_WINDOWPOS_CENTERED,
                         SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);
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

    sdl2_set_scale(1);
    SDL_RenderClear(_renderer);
    sdl2_present(_renderer);
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

float
gfx_get_scale(void)
{
    return _scale;
}

unsigned
gfx_get_color_depth(void)
{
    return 24;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, int x, int y)
{
    LOG("entry, bm: %dx%d(%d@%d) %ubpp (%u octets per scanline), x: %u, y: %u",
        bm->width, bm->height, bm->chunk_height, bm->chunk_top, bm->bpp,
        bm->opl, x, y);

    if ((1 != bm->bpp) && (4 != bm->bpp) && (32 != bm->bpp))
    {
        errno = EFTYPE;
        return false;
    }

    SDL_Surface *screen = SDL_GetWindowSurface(_window);

    int      lines = bm->chunk_height ? bm->chunk_height : abs(bm->height);
    SDL_Rect src_rect = {0, 0, bm->width, lines};
    SDL_Rect dst_rect = {x, y, _scale * bm->width, _scale * lines};
    uint32_t format = SDL_PIXELFORMAT_XRGB8888;

    SDL_Surface *surface = NULL;

    if (1 == bm->bpp)
    {
        char *bits = (char *)bm->bits;
        char *end = bits + bm->opl * lines;
        while (bits < end)
        {
            *bits = ~*bits;
            bits++;
        }

        format = SDL_PIXELFORMAT_INDEX1MSB;
    }

    if (4 == bm->bpp)
    {
        surface = SDL_CreateRGBSurfaceWithFormat(0, bm->width, lines, 32,
                                                 SDL_PIXELFORMAT_XRGB8888);
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
        surface = SDL_CreateRGBSurfaceWithFormatFrom(bm->bits, bm->width, lines,
                                                     bm->bpp, bm->opl, format);
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

    sdl2_present(_renderer);
    return true;
}

bool
gfx_draw_line(const gfx_rect *rect, gfx_color color)
{
    LOG("entry, rect: %dx%d@%u,%u, color: %d", rect->width, rect->height,
        rect->left, rect->top, color);
    _rendering_text = false;

    SDL_Rect sdl_rect = {rect->left, rect->top,
                         rect->width - ((1 == rect->height) ? 0 : 1),
                         rect->height - ((1 == rect->width) ? 0 : 1)};
    _set_color(color);

    for (int i = 0; i < _scale; i++)
    {
        SDL_RenderDrawLine(_renderer, sdl_rect.x, sdl_rect.y,
                           sdl_rect.x + sdl_rect.w, sdl_rect.y + sdl_rect.h);
        if (sdl_rect.w > sdl_rect.h)
        {
            sdl_rect.y++;
        }
        else
        {
            sdl_rect.x++;
        }
    }

    sdl2_present(_renderer);
    return true;
}

bool
gfx_draw_rectangle(const gfx_rect *rect, gfx_color color)
{
    LOG("entry, rect: %dx%d@%u,%u, color: %d", rect->width, rect->height,
        rect->left, rect->top, color);
    _rendering_text = false;

    SDL_Rect sdl_rect = {rect->left, rect->top, rect->width, rect->height};
    _set_color(color);

    for (int i = 0; i < _scale; i++)
    {
        sdl_rect.x--;
        sdl_rect.y--;
        sdl_rect.w += 2;
        sdl_rect.h += 2;
        SDL_RenderDrawRect(_renderer, &sdl_rect);
    }

    sdl2_present(_renderer);
    return true;
}

bool
gfx_fill_rectangle(const gfx_rect *rect, gfx_color color)
{
    LOG("entry, rect: %dx%d@%u,%u, color: %d", rect->width, rect->height,
        rect->left, rect->top, color);
    _rendering_text = false;

    SDL_Rect sdl_rect = {rect->left, rect->top, rect->width, rect->height};
    _set_color(color);
    SDL_RenderFillRect(_renderer, &sdl_rect);
    sdl2_present(_renderer);
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
    sdl2_present(_renderer);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
    return true;
}

bool
gfx_set_title(const char *title)
{
    SDL_SetWindowTitle(_window, title);
    return true;
}
