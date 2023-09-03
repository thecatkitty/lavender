#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>

#include <fmt/exe.h>
#include <fmt/zip.h>
#include <gfx.h>
#include <pal.h>
#include <snd.h>

#include "pal_impl.h"

typedef struct
{
    long size;
} fdata;

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

static SDL_Keycode    _keycode;
static bool           _mouse_enabled = false;
static uint16_t       _mouse_x, _mouse_y, _mouse_buttons;
static gfx_dimensions _mouse_cell;

void
pal_initialize(int argc, char *argv[])
{
    struct timespec time;
    clock_gettime(CLOCK_BOOTTIME, &time);
    _start_msec = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    LOG("entry");

    if (!zip_open(argv[0]))
    {
        LOG("cannot open the archive '%s'. %s", argv[0], strerror(errno));
        abort();
    }

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        __pal_assets[i].inzip = -1;
        __pal_assets[i].flags = 0;
        __pal_assets[i].data = NULL;
    }

    if (0 > SDL_Init(0))
    {
        LOG("cannot initialize SDL. %s", SDL_GetError());
        abort();
    }

    if (!gfx_initialize())
    {
        LOG("cannot initialize graphics");
        pal_cleanup();
        abort();
    }

    gfx_get_glyph_dimensions(&_mouse_cell);

    if (!snd_initialize())
    {
        LOG("cannot initialize sound");
    }
}

void
pal_cleanup(void)
{
    LOG("entry");

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        if (NULL != __pal_assets[i].data)
        {
            zip_free_data(__pal_assets[i].data);
        }
    }

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
