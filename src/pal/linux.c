#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <blkid/blkid.h>
#include <fontconfig/fontconfig.h>

#include <fmt/exe.h>
#include <gfx.h>
#include <pal.h>
#include <platform/sdl2arch.h>
#include <snd.h>

#include "../resource.h"
#include "pal_impl.h"

extern char _binary_obj_version_txt_start[];

extern char __w32_rsrc_start[];

static char *_font = NULL;
static long  _start_msec;

extern int
__fluid_init(void);

static void
_show_help(const char *self)
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
    _start_msec = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    LOG("entry");

    for (int i = 1; i < argc; i++)
    {
        if ('-' != argv[i][0])
        {
            continue;
        }

        if ('v' == argv[i][1])
        {
            _show_help(argv[0]);
            exit(0);
        }
    }

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

    __fluid_init();

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

bool
pal_get_machine_id(uint8_t *mid)
{
    LOG("entry");
    int fd = 0;

    if (0 > (fd = open("/var/lib/dbus/machine-id", O_RDONLY)))
    {
        LOG("exit, cannot open the file!");
        return false;
    }

    char buffer[PAL_MACHINE_ID_SIZE * 2];
    int  size = read(fd, buffer, sizeof(buffer));
    close(fd);

    if (sizeof(buffer) != size)
    {
        LOG("exit, cannot read the file!");
        return false;
    }

    if (NULL == mid)
    {
        LOG("exit, capable");
        return true;
    }

    for (int i = 0; i < PAL_MACHINE_ID_SIZE; i++)
    {
        mid[i] = xtob(buffer + (i * 2));
    }

    LOG("exit, "
        "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
        mid[0] & 0xFF, mid[1] & 0xFF, mid[2] & 0xFF, mid[3] & 0xFF,
        mid[4] & 0xFF, mid[5] & 0xFF, mid[6] & 0xFF, mid[7] & 0xFF,
        mid[8] & 0xFF, mid[9] & 0xFF, mid[10] & 0xFF, mid[11] & 0xFF,
        mid[12] & 0xFF, mid[13] & 0xFF, mid[14] & 0xFF, mid[15] & 0xFF);
    return true;
}

uint32_t
pal_get_medium_id(const char *tag)
{
    LOG("entry");
    unsigned volume_sn_high = 0, volume_sn_low = 0;

    char self[PATH_MAX];
    if (0 > readlink("/proc/self/exe", self, PATH_MAX))
    {
        LOG("exit, cannot retrieve executable path!");
        return 0;
    }

    struct stat info;
    if (0 > stat(self, &info))
    {
        LOG("exit, cannot get file info for '%s'!", self);
        return 0;
    }

    blkid_cache cache;
    if (0 > blkid_get_cache(&cache, NULL))
    {
        LOG("exit, cannot get blkid cache!");
        return 0;
    }

    char *devname = blkid_devno_to_devname(info.st_dev);
    if (NULL == devname)
    {
        LOG("exit, cannot get devname for %d:%d!", major(info.st_dev),
            minor(info.st_dev));
        return 0;
    }

    const char *label = blkid_get_tag_value(cache, "LABEL", devname);
    if (NULL == label)
    {
        LOG("cannot get label of '%s'!", devname);
        goto end;
    }

    if (0 != strcmp(tag, label))
    {
        LOG("label '%s' not matching '%s'", label, tag);
        goto end;
    }

    const char *uuid = blkid_get_tag_value(cache, "UUID", devname);
    if (NULL == uuid)
    {
        LOG("cannot get UUID of '%s'!", devname);
        goto end;
    }

    LOG("dev %d:%d [%s] '%s' - uuid: %s", major(info.st_dev),
        minor(info.st_dev), devname, label, uuid);

    if (2 != sscanf(uuid, "%04X-%04X", &volume_sn_high, &volume_sn_low))
    {
        LOG("unknown volume serial number format!");
        volume_sn_high = volume_sn_low = 0;
        goto end;
    }

end:
    LOG("exit, %04X-%04X", volume_sn_high, volume_sn_low);
    free(devname);
    return (volume_sn_high << 16) | volume_sn_low;
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
