#include <ctype.h>

#include <SDL2/SDL.h>

#include <gfx.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <snd.h>

#include "evtmouse.h"

static SDL_Keycode    _keycode;
static gfx_dimensions _mouse_cell;
static SDL_Renderer  *_renderer = NULL;

bool
sdl2arch_initialize(void)
{
    if (0 > SDL_Init(0))
    {
        LOG("cannot initialize SDL. %s", SDL_GetError());
        return false;
    }

    if (!gfx_initialize())
    {
        LOG("cannot initialize graphics");
        return false;
    }

    gfx_get_glyph_dimensions(&_mouse_cell);
    return true;
}

void
sdl2arch_cleanup(void)
{
    LOG("entry");

    snd_cleanup();
    gfx_cleanup();

    SDL_Quit();
}

void
sdl2arch_present(SDL_Renderer *renderer)
{
    _renderer = renderer;
}

bool
pal_handle(void)
{
    if (_renderer)
    {
        SDL_RenderPresent(_renderer);
        _renderer = NULL;
    }

    SDL_PumpEvents();

    SDL_Event e;
    if (1 > SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    {
        return true;
    }

    switch (e.type)
    {
    case SDL_QUIT: {
        LOG("quit");
        return false;
    }

    case SDL_KEYDOWN: {
        LOG("key '%s' down", SDL_GetKeyName(e.key.keysym.sym));
        _keycode = e.key.keysym.sym;
        break;
    }

    case SDL_KEYUP:
        LOG("key '%s' up", SDL_GetKeyName(e.key.keysym.sym));
        _keycode = 0;
        break;

    case SDL_MOUSEMOTION: {
        LOG("mouse x: %d, y: %d", e.motion.x, e.motion.y);
        _mouse_x = e.motion.x / _mouse_cell.width;
        _mouse_y = e.motion.y / _mouse_cell.height;
        break;
    }

    case SDL_MOUSEBUTTONDOWN: {
        if (!_mouse_enabled)
        {
            break;
        }

        LOG("mouse button %u down", e.button.button);
        if (SDL_BUTTON_LEFT == e.button.button)
        {
            _mouse_buttons |= PAL_MOUSE_LBUTTON;
        }
        else if (SDL_BUTTON_RIGHT == e.button.button)
        {
            _mouse_buttons |= PAL_MOUSE_RBUTTON;
        }
        break;
    }

    case SDL_MOUSEBUTTONUP: {
        LOG("mouse button %u up", e.button.button);
        if (SDL_BUTTON_LEFT == e.button.button)
        {
            _mouse_buttons &= ~PAL_MOUSE_LBUTTON;
        }
        else if (SDL_BUTTON_RIGHT == e.button.button)
        {
            _mouse_buttons &= ~PAL_MOUSE_RBUTTON;
        }
        break;
    }
    }

    return true;
}

uint16_t
pal_get_keystroke(void)
{
    if (!_keycode)
    {
        return 0;
    }

    int c = _keycode;
    _keycode = 0;

    switch (c)
    {
    case '-':
        c = VK_OEM_MINUS;
        break;
    case SDLK_DELETE:
        c = VK_DELETE;
        break;
    case SDLK_RIGHT:
        c = VK_RIGHT;
        break;
    case SDLK_LEFT:
        c = VK_LEFT;
        break;
    case SDLK_DOWN:
        c = VK_DOWN;
        break;
    case SDLK_UP:
        c = VK_UP;
        break;
    }
    
    if (255 < c)
    {
        c = 0;
    }

    if (islower(c))
    {
        c = toupper(c);
    }

    LOG("keystroke: %#.2x", c);
    return c;
}
