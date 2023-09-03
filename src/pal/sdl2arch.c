#include <SDL2/SDL.h>

#include <gfx.h>
#include <pal.h>
#include <snd.h>

static SDL_Keycode    _keycode;
static bool           _mouse_enabled = false;
static uint16_t       _mouse_x, _mouse_y, _mouse_buttons;
static gfx_dimensions _mouse_cell;

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
pal_handle(void)
{
    SDL_PumpEvents();

    SDL_Event e;
    if (1 > SDL_PeepEvents(&e, 1, SDL_GETEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT))
    {
        return;
    }

    switch (e.type)
    {
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
        if (!_mouse_enabled)
        {
            break;
        }

        LOG("mouse x: %d, y: %d", e.motion.x, e.motion.y);
        _mouse_x = e.motion.x / _mouse_cell.width;
        _mouse_y = e.motion.y / 16;
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
        if (!_mouse_enabled)
        {
            break;
        }

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

void
pal_enable_mouse(void)
{
    LOG("entry");

    _mouse_enabled = true;
    return;
}

void
pal_disable_mouse(void)
{
    LOG("entry");

    _mouse_enabled = false;
    return;
}

uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y)
{
    if (!_mouse_enabled)
    {
        return 0;
    }

    *x = _mouse_x;
    *y = _mouse_y;
    if (_mouse_buttons)
    {
        LOG("x: %u, y: %u, buttons: %#x", *x, *y, _mouse_buttons);
    }
    return _mouse_buttons;
}
