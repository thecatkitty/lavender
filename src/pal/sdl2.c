#include <ctype.h>

#include <SDL2/SDL.h>

#include <arch/sdl2.h>
#include <gfx.h>
#include <pal.h>
#include <snd.h>

#include "evtmouse.h"

static SDL_Keycode    _keycode;
static gfx_dimensions _mouse_cell;
static SDL_Renderer  *_renderer = NULL;

bool
sdl2_initialize(void)
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
sdl2_cleanup(void)
{
    LOG("entry");

#if defined(CONFIG_SOUND)
    snd_cleanup();
#endif // CONFIG_SOUND

    gfx_cleanup();

    SDL_Quit();
}

void
sdl2_present(SDL_Renderer *renderer)
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
        if (KMOD_CTRL & e.key.keysym.mod)
        {
            if ((SDLK_PLUS == e.key.keysym.sym) ||
                (SDLK_KP_PLUS == e.key.keysym.sym))
            {
                sdl2_set_scale(gfx_get_scale() + 1);
                gfx_get_glyph_dimensions(&_mouse_cell);
                break;
            }

            if ((SDLK_MINUS == e.key.keysym.sym) ||
                (SDLK_KP_MINUS == e.key.keysym.sym))
            {
                sdl2_set_scale(gfx_get_scale() - 1);
                gfx_get_glyph_dimensions(&_mouse_cell);
                break;
            }
        }

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
    case SDLK_PAGEUP:
        c = VK_PRIOR;
        break;
    case SDLK_F1:
        c = VK_F1;
        break;
    case SDLK_F2:
        c = VK_F2;
        break;
    case SDLK_F3:
        c = VK_F3;
        break;
    case SDLK_F4:
        c = VK_F4;
        break;
    case SDLK_F5:
        c = VK_F5;
        break;
    case SDLK_F6:
        c = VK_F6;
        break;
    case SDLK_F7:
        c = VK_F7;
        break;
    case SDLK_F8:
        c = VK_F8;
        break;
    case SDLK_F9:
        c = VK_F9;
        break;
    case SDLK_F10:
        c = VK_F10;
        break;
    case SDLK_F11:
        c = VK_F11;
        break;
    case SDLK_F12:
        c = VK_F12;
        break;
    case SDLK_KP_0:
        c = '0';
        break;
    case SDLK_KP_1:
        c = '1';
        break;
    case SDLK_KP_2:
        c = '2';
        break;
    case SDLK_KP_3:
        c = '3';
        break;
    case SDLK_KP_4:
        c = '4';
        break;
    case SDLK_KP_5:
        c = '5';
        break;
    case SDLK_KP_6:
        c = '6';
        break;
    case SDLK_KP_7:
        c = '7';
        break;
    case SDLK_KP_8:
        c = '8';
        break;
    case SDLK_KP_9:
        c = '9';
        break;
    default:
        if (255 < c)
        {
            c = 0;
        }

        if (islower(c))
        {
            c = toupper(c);
        }
    }

    LOG("keystroke: %#.2x", c);
    return c;
}
