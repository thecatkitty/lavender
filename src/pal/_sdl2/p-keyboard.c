#include <pal.h>

#include "impl.h"

SDL_Keycode sdl2_keycode;

uint16_t
pal_get_keystroke(void)
{
    if (!sdl2_keycode)
    {
        return 0;
    }

    int c = sdl2_keycode;
    sdl2_keycode = 0;

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
