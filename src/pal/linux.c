#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <fontconfig/fontconfig.h>

#include <fmt/exe.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <snd.h>

#include "pal_impl.h"

extern char _binary_obj_version_txt_start[];

// FIXME: W/A for https://sourceware.org/bugzilla/show_bug.cgi?id=30719
#ifndef __x86_64__
extern
#endif
    char __w32_rsrc_start[]
#ifdef __x86_64__
    __attribute__((section(".rsrc"))) = {}
#endif
;

static char *_font = NULL;
static long  _start_msec;

void
pal_initialize(int argc, char *argv[])
{
    struct timespec time;
    clock_gettime(CLOCK_BOOTTIME, &time);
    _start_msec = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    LOG("entry");

    if (!ziparch_initialize(argv[0]))
    {
        LOG("ZIP architecture initialization failed");
        abort();
    }

    if (!sdl2arch_initialize())
    {
        LOG("SDL2 architecture initialization failed");
        pal_cleanup();
        abort();
    }

    if (!snd_initialize(NULL))
    {
        LOG("cannot initialize sound");
    }
}

void
pal_cleanup(void)
{
    LOG("entry");

    if (_font)
    {
        free(_font);
    }

    sdl2arch_cleanup();
    ziparch_cleanup();
}

uint32_t
pal_get_counter(void)
{
    struct timespec time;
    clock_gettime(CLOCK_BOOTTIME, &time);

    long time_msec = time.tv_sec * 1000 + time.tv_nsec / 1000000;
    return time_msec - _start_msec;
}

uint32_t
pal_get_ticks(unsigned ms)
{
    return ms;
}

void
pal_sleep(unsigned ms)
{
    LOG("entry, ms: %u", ms);

    usleep(ms);
}

const char *
pal_get_version_string(void)
{
    return _binary_obj_version_txt_start;
}

uint32_t
pal_get_medium_id(const char *tag)
{
    LOG("entry");
    return 0;
}

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    LOG("entry, id: %u, buffer: %p, max_length: %d", id, buffer, max_length);

    int length = exe_pe_load_string(__w32_rsrc_start, id, buffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);

        LOG("exit, string missing");
        return sizeof(msg) - 1;
    }

    LOG("exit, '%s'", buffer);
    return length;
}

void
pal_alert(const char *text, int error)
{
    puts("\n=====");
    puts(text);
    if (0 != error)
    {
        printf("errno %d, %s\n", error, strerror(error));
    }
}

const char *
sdl2arch_get_font(void)
{
    if (_font)
    {
        return _font;
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

    _font = malloc(strlen((const char *)font) + 1);
    if (NULL == _font)
    {
        LOG("cannot allocate buffer");
        FcPatternDestroy(match);
        return NULL;
    }

    strcpy(_font, (const char *)font);
    FcPatternDestroy(match);
    LOG("matched: '%s'", _font);
    return _font;
}
