#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
#include <unistd.h>

#include <blkid/blkid.h>
#include <fontconfig/fontconfig.h>

#include <arch/sdl2.h>
#include <fmt/exe.h>
#include <gfx.h>
#include <net.h>
#include <pal.h>
#include <snd.h>

#include "../resource.h"
#include "pal_impl.h"

extern char _binary_obj_version_txt_start[];

static char *_font = NULL;
static long  _start_msec;
static char  _state_dir[PATH_MAX] = "";

#if defined(CONFIG_SOUND)
static bool _beepemu = false;

extern int
__fluid_init(void);
#endif // CONFIG_SOUND

static char *
_get_font(void);

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

#if defined(CONFIG_SOUND)
        if ('b' == argv[i][1])
        {
            _beepemu = true;
        }
#endif // CONFIG_SOUND
    }

    if (!ziparch_initialize(argv[0]))
    {
        LOG("ZIP architecture initialization failed");
        abort();
    }

    _font = _get_font();
    if (!sdl2_initialize(_font))
    {
        LOG("SDL2 architecture initialization failed");
        pal_cleanup();
        abort();
    }

#if defined(CONFIG_SOUND)
    __fluid_init();

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

    if (_font)
    {
        free(_font);
    }

    net_stop();
    sdl2_cleanup();
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

static void
_mkdir_recursive(const char *path)
{
    LOG("entry, '%s'", path);

    char   temp_path[PATH_MAX] = "";
    size_t len = strlen(path);

    strncpy(temp_path, path, PATH_MAX);
    temp_path[len] = '\0';

    for (char *part = temp_path + 1; *part; part++)
    {
        if (*part == '/')
        {
            *part = '\0'; // Terminate the string
            if ((0 != mkdir(temp_path, 0700)) && (EEXIST != errno))
            {
                LOG("exit, cannot create '%s'", temp_path);
                return;
            }
            *part = '/'; // Restore the slash
        }
    }

    if ((0 != mkdir(temp_path, 0700)) && (EEXIST != errno))
    {
        LOG("exit, cannot create '%s'", temp_path);
        return;
    }

    LOG("exit, ok");
    return;
}

static bool
_retrieve_state_dir(void)
{
    LOG("entry");

    if (0 != _state_dir[0])
    {
        LOG("exit, already known: '%s'", _state_dir);
        return true;
    }

    const char *xdg_state_home = getenv("XDG_STATE_HOME");
    if (NULL != xdg_state_home)
    {
        strncpy(_state_dir, xdg_state_home, PATH_MAX);
    }
    else
    {
        const char *home = getenv("HOME");
        if (home == NULL)
        {
            LOG("exit, unknown $HOME");
            return false;
        }

        snprintf(_state_dir, PATH_MAX, "%s/.local/state", home);
    }

    strncat(_state_dir, "/celones/lavender", PATH_MAX - strlen(_state_dir));
    LOG("exit, '%s'", _state_dir);
    return true;
}

static bool
_concat_path(char *path, const char *parent, const char *child)
{
    size_t parent_len = strlen(parent);
    size_t child_len = strlen(child);

    if (PATH_MAX <= (parent_len + child_len + 2))
    {
        return false;
    }

    strncpy(path, parent, PATH_MAX - 1);
    path[PATH_MAX - 1] = '\0';

    if (parent[parent_len - 1] != '/')
    {
        strncat(path, "/", PATH_MAX - strlen(path) - 1);
    }

    strncat(path, child, PATH_MAX - strlen(path) - 1);
    return true;
}

size_t
pal_load_state(const char *name, uint8_t *buffer, size_t size)
{
    LOG("entry, '%s'", name);

    if (!_retrieve_state_dir())
    {
        LOG("exit, unknown state directory");
        return 0;
    }

    char path[PATH_MAX];
    if (!_concat_path(path, _state_dir, name))
    {
        LOG("exit, path too long");
        return 0;
    }

    int fd;
    if (0 > (fd = open(path, O_RDONLY)))
    {
        LOG("exit, cannot open '%s'", path);
        return 0;
    }

    int byte_count;
    if (0 >= (byte_count = read(fd, buffer, size)))
    {
        LOG("exit, cannot read '%s'", path);
        close(fd);
        return 0;
    }

    close(fd);
    LOG("exit, %d bytes read", byte_count);
    return (size_t)byte_count;
}

bool
pal_save_state(const char *name, const uint8_t *buffer, size_t size)
{
    LOG("entry, '%s'", name);

    if (!_retrieve_state_dir())
    {
        LOG("exit, unknown state directory");
        return false;
    }
    _mkdir_recursive(_state_dir);

    char path[PATH_MAX];
    if (!_concat_path(path, _state_dir, name))
    {
        LOG("exit, path too long");
        return 0;
    }

    int fd;
    if (0 > (fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600)))
    {
        LOG("exit, cannot open '%s'", path);
        return false;
    }

    if (size != write(fd, buffer, size))
    {
        LOG("exit, cannot write '%s'", path);
        close(fd);
        return false;
    }

    close(fd);
    LOG("exit, %d bytes written", size);
    return true;
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

static const char *OPEN_COMMAND[] = {
    "xdg-open",
    "x-www-browser",
    "sensible-browser",
    "gio open",
};

void
pal_open_url(const char *url)
{
    LOG("entry, '%s'", url);

    char cmd[PATH_MAX];

    for (int i = 0; i < lengthof(OPEN_COMMAND); i++)
    {
        LOG("trying %s", OPEN_COMMAND[i]);
        snprintf(cmd, PATH_MAX, "%s \"%s\"", OPEN_COMMAND[i], url);
        if (0 == system(cmd))
        {
            LOG("exit");
            return;
        }
    }

    LOG("exit, failed");
}

static char *
_get_font(void)
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

bool
linux_beepemu_enabled(void)
{
    LOG(_beepemu ? "yes" : "no");
    return _beepemu;
}
