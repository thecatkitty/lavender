#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arch/dos.h>
#include <arch/dos/bios.h>
#include <arch/dos/msdos.h>
#include <arch/dos/msmouse.h>
#include <arch/dos/winoldap.h>
#include <arch/zip.h>
#include <fmt/exe.h>
#include <fmt/zip.h>
#include <gfx.h>
#include <pal.h>
#include <snd.h>

#ifdef CONFIG_ANDREA
#include <andrea.h>
#endif

#include "../../resource.h"
#include "hw.h"
#include "impl.h"

#ifdef CONFIG_ANDREA
#define MAX_DRIVERS 4
#endif

gfx_glyph_data dos_font = NULL;

extern int ddcall
__cga_init(void);

extern char __edata[], __sbss[], __ebss[];

#ifdef STACK_PROFILING
static uint64_t      *stack_start_;
static uint64_t      *stack_end_ = (uint64_t *)0xFFE8;
static const uint64_t STACK_FILL_PATTERN = 0x0123456789ABCDEFULL;
#endif // STACK_PROFILING

#ifdef ZIP_PIGGYBACK
static zip_cdir_end_header *
locate_cdir(void *from, void *to)
{
    const char *ptr = (char *)to - sizeof(zip_cdir_end_header);
    while (ptr >= (char *)from)
    {
        zip_cdir_end_header *cdir = (zip_cdir_end_header *)ptr;
        if ((ZIP_PK_SIGN == cdir->pk_signature) &&
            (ZIP_CDIR_END_SIGN == cdir->header_signature))
        {
            return cdir;
        }

        ptr--;
    }

    return NULL;
}
#endif

static void
die_incompatible(void)
{
    __cga_init();
    gfx_get_font_data(&dos_font);

    char msg[GFX_COLUMNS];
    pal_load_string(IDS_UNSUPPENV, msg, sizeof(msg));
    utf8_encode(msg, msg, pal_wctoa);
    msg[strlen(msg)] = '$';
    msdos_puts(msg);
}

static bool
is_dos_major(uint8_t major)
{
    uint8_t dos_major = msdos_get_version() & 0xFF;
    return (1 == major) ? (0 == dos_major) : (major == dos_major);
}

static bool
is_winnt(void)
{
    // NTVDM claims to be DOS 5
    if (!is_dos_major(5))
    {
        return false; // It's not 5, so it's not NTVDM.
    }

    // Does it have %OS% environment variable?
    const char *os = getenv("OS");
    if (NULL == os)
    {
        return false; // It's just a regular DOS 5.
    }

    // Does it claim to be Windows NT?
    return 0 == strcmp(os, "Windows_NT");
}

static uint16_t
get_winnt_version(void)
{
    const char *sysroot = getenv("SYSTEMROOT");
    if (NULL == sysroot)
    {
        return 0; // No system root
    }

    char smss[261];
    strcpy(smss, sysroot);
    strcat(smss, "\\system32\\smss.exe");

    int fd;
    if (0 > (fd = open(smss, O_RDONLY)))
    {
        return 0; // No SMSS
    }

    union {
        exe_dos_header         dos_header;
        ULONG                  new_signature;
        exe_pe_optional_header optional_header;
    } data;

    read(fd, &data, sizeof(exe_dos_header));
    if (0x5A4D != data.dos_header.e_magic)
    {
        return 0; // Invalid executable
    }

    lseek(fd, data.dos_header.e_lfanew, SEEK_SET);
    read(fd, &data, sizeof(ULONG));
    if (0x00004550 != data.new_signature)
    {
        return 0; // Invalid executable
    }

    lseek(fd, sizeof(exe_pe_file_header), SEEK_CUR);
    read(fd, &data, sizeof(exe_pe_optional_header));
    close(fd);

    return (data.optional_header.MajorImageVersion << 8) |
           data.optional_header.MinorImageVersion;
}

static bool
is_compatible(void)
{
    // We're using DOS 2.0+ API here, and we can't run on Vista and above.
    return !is_dos_major(1) && (!is_winnt() || (0x0600 > get_winnt_version()));
}

#ifdef CONFIG_ANDREA
static bool
driver_enum_callback(const char *name, void *data)
{
    uint16_t *drivers = (uint16_t *)data;

    for (size_t i = 0; i < MAX_DRIVERS; i++)
    {
        if (0 == drivers[i])
        {
            drivers[i] = dos_load_driver(name);
            return true;
        }
    }

    return false;
}
#endif

#if defined(CONFIG_SOUND)
static bool
snddev_enum_callback(device *dev, void *data)
{
    fputs("  ", stdout);
    fputs(dev->name, stdout);
    for (int i = strlen(dev->name); i < 8; i++)
    {
        fputc(' ', stdout);
    }
    fputs(dev->description, stdout);
    fputc('\n', stdout);
    fflush(stdout);
    return true;
}
#endif // CONFIG_SOUND

static void
show_help(const char *self)
{
#if defined(CONFIG_SOUND)
#if defined(CONFIG_ANDREA)
    uint16_t drivers[MAX_DRIVERS] = {0};
    pal_enum_assets(driver_enum_callback, "snd*.sys", drivers);
#endif
    snd_load_inbox_drivers();
#endif // CONFIG_SOUND

    const char *name = strrchr(self, '\\');
    if (NULL == name)
    {
        name = self;
    }
    else
    {
        name++;
    }

    __cga_init();
    gfx_get_font_data(&dos_font);

    char msgu[GFX_COLUMNS], msga[GFX_COLUMNS];
    puts(pal_get_version_string());

    pal_load_string(IDS_DESCRIPTION, msgu, sizeof(msgu));
    utf8_encode(msgu, msga, pal_wctoa);
    puts(msga);
    puts("");

    fputs(name ? name : "LAVENDER", stdout);
    printf(" [/?");
#if defined(CONFIG_SOUND)
    printf(" | /S<dev>");
#endif // CONFIG_SOUND
    puts("]");

#if defined(CONFIG_SOUND)
    puts("\ndev:");
    snd_enum_devices(snddev_enum_callback, NULL);
#endif // CONFIG_SOUND

    puts("");
    pal_load_string(IDS_COPYRIGHT, msgu, sizeof(msgu));
    utf8_encode(msgu, msga, pal_wctoa);
    puts(msga);

    puts("\nhttps://celones.pl/lavender");

#if defined(CONFIG_ANDREA)
    for (size_t i = 0; i < MAX_DRIVERS; i++)
    {
        if (0 != drivers[i])
        {
            dos_unload_driver(drivers[i]);
        }
    }
#endif
}

void
pal_initialize(int argc, char *argv[])
{
#ifdef STACK_PROFILING
    stack_start_ = (uint64_t *)((unsigned)__ebss / 8 * 8) + 1;
    for (uint64_t *ptr = stack_start_; ptr < stack_end_; ptr++)
        *ptr = STACK_FILL_PATTERN;
#endif // STACK_PROFILING

    bool arg_help = false;
#if defined(CONFIG_SOUND)
    const char *arg_snd = NULL;
#endif // CONFIG_SOUND

    for (int i = 1; i < argc; i++)
    {
        if ('/' != argv[i][0])
        {
            continue;
        }

#if defined(CONFIG_SOUND)
        if ('s' == tolower(argv[i][1]))
        {
            arg_snd = argv[i] + 2;
        }
#endif // CONFIG_SOUND

        if ('?' == argv[i][1])
        {
            arg_help = true;
        }
    }

#ifdef ZIP_PIGGYBACK
    zip_cdir_end_header *cdir = locate_cdir(__edata, __sbss);
    if (NULL == cdir)
    {
        char msg[GFX_COLUMNS];
        pal_load_string(IDS_ERROR, msg, sizeof(msg));
        pal_load_string(IDS_NOARCHIVE, msg + strlen(msg),
                        sizeof(msg) - strlen(msg));
        pal_alert(msg, 0);
        msdos_exit(1);
    }

    if (!ziparch_initialize(cdir))
#else
    if (is_dos_major(2))
    {
        msdos_puts("EXE \x1A DOS 3+\r\n$");
        die_incompatible();
        msdos_exit(1);
    }

#ifdef CONFIG_IA16X
    if (0x200 > dosxm_init())
    {
        msdos_puts("XMS \x1A HIMEM\r\n$");
        die_incompatible();
        msdos_exit(1);
    }

    dos_initialize_cache();
#endif

    if (!ziparch_initialize(argv[0]))
#endif
    {
        char msg[GFX_COLUMNS];
        pal_load_string(IDS_ERROR, msg, sizeof(msg));
        pal_load_string(IDS_NOARCHIVE, msg + strlen(msg),
                        sizeof(msg) - strlen(msg));
        pal_alert(msg, errno);
        msdos_exit(1);
    }

#ifdef CONFIG_ANDREA
    andrea_init();
#endif

    if (arg_help)
    {
        show_help(argv[0]);
        msdos_exit(1);
    }

    if (!is_compatible())
    {
        hasset support = pal_open_asset("support.txt", O_RDONLY);
        if (NULL == support)
        {
            die_incompatible();
            msdos_exit(1);
        }

        char *data = pal_load_asset(support);
        if (NULL == data)
        {
            msdos_exit(1);
        }

        msdos_puts(data);
        bios_get_keystroke();
        msdos_exit(1);
    }

    pit_initialize();
    if (!gfx_initialize())
    {
        pal_alert("", errno);
        pit_cleanup();
    }

    gfx_get_font_data(&dos_font);
    gfx_get_glyph_dimensions(&dos_cell);
    dos_mouse = msmouse_init();

#ifdef CONFIG_IA16X
    if (dos_is_windows())
    {
        winoldap_set_closable(1);
    }
#endif

#if defined(CONFIG_SOUND)
    snd_initialize(arg_snd);
#endif // CONFIG_SOUND
}

void
pal_cleanup(void)
{
    ziparch_cleanup();
#ifdef CONFIG_IA16X
    dos_cleanup_cache();
#endif
    gfx_cleanup();

#if defined(CONFIG_SOUND)
    snd_cleanup();
#endif // CONFIG_SOUND

    pit_cleanup();
    nosound();

#ifdef STACK_PROFILING
    uint64_t *untouched;
    for (untouched = stack_start_; untouched < stack_end_; untouched++)
    {
        if (STACK_FILL_PATTERN != *untouched)
            break;
    }

    int stack_size = (int)(0x10000UL - (uint16_t)untouched);

    msdos_puts("Stack usage: $");

    char buffer[6];
    itoa(stack_size, buffer, 10);
    for (int i = 0; buffer[i]; i++)
        msdos_putc(buffer[i]);

    msdos_puts("\r\n$");
#endif // STACK_PROFILING
}
