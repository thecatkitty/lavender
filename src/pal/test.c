#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

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
extern char __w32_rsrc_start[];

long           _start_msec;
struct termios _old_termios;

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

    struct termios new_termios;

    tcgetattr(STDIN_FILENO, &_old_termios);
    memcpy(&new_termios, &_old_termios, sizeof(new_termios));

    cfmakeraw(&new_termios);
    new_termios.c_oflag |= ONLCR | OPOST;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
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

    tcsetattr(0, TCSANOW, &_old_termios);
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
    struct timeval tv = {0L, 0L};

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    if (0 == select(1, &fds, NULL, NULL, &tv))
    {
        return 0;
    }

    char c;
    if (0 > read(STDIN_FILENO, &c, sizeof(c)))
    {
        return 0;
    }

    if (0x7F == c)
    {
        c = VK_BACK;
    }

    LOG("keystroke: %#.2x", c);
    return c;
}

void
pal_enable_mouse(void)
{
    LOG("entry");
    return;
}

void
pal_disable_mouse(void)
{
    LOG("entry");
    return;
}

uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y)
{
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

bool
gfx_initialize(void)
{
    LOG("entry");
    return true;
}

void
gfx_cleanup(void)
{
    LOG("entry");
}

void
gfx_get_screen_dimensions(gfx_dimensions *dim)
{
    LOG("entry");

    dim->width = 640;
    dim->height = 200;

    LOG("exit, %dx%d", dim->width, dim->height);
}

uint16_t
gfx_get_pixel_aspect(void)
{
    LOG("entry");

    uint16_t ratio = 64 * (1);

    LOG("exit, ratio: 1:%.2f", (float)ratio / 64.0f);
    return ratio;
}

bool
gfx_draw_bitmap(gfx_bitmap *bm, uint16_t x, uint16_t y)
{
    LOG("entry, bm: %ux%u %ubpp (%u planes, %u octets per scanline), x: %u,"
        " y: %u",
        bm->width, bm->height, bm->bpp, bm->planes, bm->opl, x, y);

    return true;
}

bool
gfx_draw_line(gfx_dimensions *dim, uint16_t x, uint16_t y, gfx_color color)
{
    LOG("entry, dim: %dx%d, x: %u, y: %u, color: %d", dim->width, dim->height,
        x, y, color);

    return true;
}

bool
gfx_draw_rectangle(gfx_dimensions *rect,
                   uint16_t        x,
                   uint16_t        y,
                   gfx_color       color)
{
    LOG("entry, dim: %dx%d, x: %u, y: %u, color: %d", rect->width, rect->height,
        x, y, color);

    return true;
}

bool
gfx_fill_rectangle(gfx_dimensions *rect,
                   uint16_t        x,
                   uint16_t        y,
                   gfx_color       color)
{
    LOG("entry, dim: %dx%d, x: %u, y: %u, color: %d", rect->width, rect->height,
        x, y, color);

    return true;
}

bool
gfx_draw_text(const char *str, uint16_t x, uint16_t y)
{
    LOG("entry, str: '%s', x: %u, y: %u", str, x, y);

    return true;
}

char
gfx_wctob(uint16_t wc)
{
    return (0x80 > wc) ? wc : '?';
}

void
snd_send(const char *msg, size_t length)
{
    LOG("entry, length: %zd", length);
}
