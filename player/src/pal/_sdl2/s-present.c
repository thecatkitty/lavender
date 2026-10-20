
#include <arch/sdl2.h>

#include "impl.h"

SDL_Renderer *sdl2_renderer;

void
sdl2_present(void *renderer)
{
    sdl2_renderer = (SDL_Renderer *)renderer;
}
