#include <stdio.h>
#include <time.h>

#include <arch/sdl2.h>
#include <arch/zip.h>
#include <gfx.h>
#include <net.h>
#include <pal.h>
#include <snd.h>

#include <fontconfig/fontconfig.h>

#include "../../resource.h"
#include "impl.h"

static char *font_ = NULL;

static char *
find_font(void)
{
    if (font_)
    {
        return font_;
    }

    FcPattern *pattern = FcPatternCreate();
    FcPatternAddString(pattern, FC_FAMILY, (const FcChar8 *)"monospace");
    FcConfigSubstitute(NULL, pattern, FcMatchPattern);
    FcDefaultSubstitute(pattern);

    FcResult   result = FcResultNoMatch;
    FcPattern *match = FcFontMatch(NULL, pattern, &result);
    FcPatternDestroy(pattern);

    FcChar8 *font;
    if ((NULL == match) ||
        (FcResultMatch != FcPatternGetString(match, FC_FILE, 0, &font)))
    {
        LOG("cannot match font");
        return NULL;
    }

    font_ = malloc(strlen((const char *)font) + 1);
    if (NULL == font_)
    {
        LOG("cannot allocate buffer");
        FcPatternDestroy(match);
        return NULL;
    }

    strcpy(font_, (const char *)font);
    FcPatternDestroy(match);
    LOG("matched: '%s'", font_);
    return font_;
}

static void
show_help(const char *self)
{
    char msg[GFX_COLUMNS];
    puts(pal_get_version_string());

    pal_load_string(IDS_DESCRIPTION, msg, sizeof(msg));
    puts(msg);
    puts("");

    pal_load_string(IDS_COPYRIGHT, msg, sizeof(msg));
    puts(msg);

    puts("\n\nhttps://celones.pl/lavender");
}

void
pal_initialize(int argc, char *argv[])
{
    struct timespec time;
    clock_gettime(CLOCK_BOOTTIME, &time);
    linux_start_msec = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    LOG("entry");

    const char *arg_archive = argv[0];

    for (int i = 1; i < argc; i++)
    {
        if ('-' != argv[i][0])
        {
            if (argv[0] == arg_archive)
            {
                arg_archive = argv[i];
            }
            continue;
        }

        if ('v' == argv[i][1])
        {
            show_help(argv[0]);
            exit(0);
        }

#if defined(CONFIG_SOUND)
        if ('b' == argv[i][1])
        {
            linux_has_beepemu = true;
        }
#endif // CONFIG_SOUND
    }

    if (!ziparch_initialize(arg_archive))
    {
        LOG("ZIP architecture initialization failed");
        abort();
    }

    font_ = find_font();
    if (!sdl2_initialize(font_))
    {
        LOG("SDL2 architecture initialization failed");
        pal_cleanup();
        abort();
    }

#if defined(CONFIG_SOUND)
    if (!snd_initialize(NULL))
    {
        LOG("cannot initialize sound");
    }
#endif // CONFIG_SOUND
}

void
pal_cleanup(void)
{
    LOG("entry");

    if (font_)
    {
        free(font_);
    }

    net_stop();
    sdl2_cleanup();
    ziparch_cleanup();
}
