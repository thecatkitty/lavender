#include <pal.h>
#include <snd.h>

#include "impl.h"

bool
sdl2_initialize(const char *font)
{
    if (0 > SDL_Init(0))
    {
        LOG("cannot initialize SDL. %s", SDL_GetError());
        return false;
    }

    sdl2_font = font;
    if (!gfx_initialize())
    {
        LOG("cannot initialize graphics");
        return false;
    }

    gfx_get_glyph_dimensions(&sdl2_cell);
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
