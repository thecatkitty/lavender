#include <time.h>

#include <SDL2/SDL.h>

#include <fmt/exe.h>
#include <pal.h>
#include <snd.h>
#include <unistd.h>

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

static long _start_msec;

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

    if (!snd_initialize())
    {
        LOG("cannot initialize sound");
    }
}

void
pal_cleanup(void)
{
    LOG("entry");

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
