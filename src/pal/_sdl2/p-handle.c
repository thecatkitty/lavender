#include <arch/sdl2.h>
#include <pal.h>

#include "impl.h"
#include <evtmouse.h>

gfx_dimensions sdl2_cell;

bool
pal_handle(void)
{
    if (sdl2_renderer)
    {
        SDL_RenderPresent(sdl2_renderer);
        sdl2_renderer = NULL;
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
                gfx_get_glyph_dimensions(&sdl2_cell);
                break;
            }

            if ((SDLK_MINUS == e.key.keysym.sym) ||
                (SDLK_KP_MINUS == e.key.keysym.sym))
            {
                sdl2_set_scale(gfx_get_scale() - 1);
                gfx_get_glyph_dimensions(&sdl2_cell);
                break;
            }
        }

        sdl2_keycode = e.key.keysym.sym;
        break;
    }

    case SDL_KEYUP:
        LOG("key '%s' up", SDL_GetKeyName(e.key.keysym.sym));
        sdl2_keycode = 0;
        break;

    case SDL_MOUSEMOTION: {
        LOG("mouse x: %d, y: %d", e.motion.x, e.motion.y);
        evtmouse_set_position(e.motion.x / sdl2_cell.width,
                              e.motion.y / sdl2_cell.height);
        break;
    }

    case SDL_MOUSEBUTTONDOWN: {
        LOG("mouse button %u down", e.button.button);
        if (SDL_BUTTON_LEFT == e.button.button)
        {
            evtmouse_press(PAL_MOUSE_LBUTTON);
        }
        else if (SDL_BUTTON_RIGHT == e.button.button)
        {
            evtmouse_press(PAL_MOUSE_RBUTTON);
        }
        break;
    }

    case SDL_MOUSEBUTTONUP: {
        LOG("mouse button %u up", e.button.button);
        if (SDL_BUTTON_LEFT == e.button.button)
        {
            evtmouse_release(PAL_MOUSE_LBUTTON);
        }
        else if (SDL_BUTTON_RIGHT == e.button.button)
        {
            evtmouse_release(PAL_MOUSE_RBUTTON);
        }
        break;
    }
    }

    return true;
}
