#include <conio.h>
#include <ctype.h>
#include <fcntl.h>
#include <libi86/stdlib.h>
#include <libi86/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <api/bios.h>
#include <api/dos.h>
#include <api/msmouse.h>
#include <fmt/exe.h>
#include <fmt/fat.h>
#include <fmt/utf8.h>
#include <fmt/zip.h>
#include <gfx.h>
#include <pal.h>
#include <platform/dospc.h>
#include <snd.h>

#ifdef CONFIG_ANDREA
#include <andrea.h>
#endif

#include "../resource.h"
#include "dospc.h"
#include "pal_impl.h"
#include "../gfx/glyph.h"

#define TEXT_COLUMNS 80
#define TEXT_LINES   25

#ifdef CONFIG_ANDREA
#define MAX_DRIVERS 4

typedef int ddcall (*pf_drvinit)(void);
typedef int ddcall (*pf_drvdeinit)(void);
#endif

extern char __edata[], __sbss[], __ebss[];
extern char _binary_obj_version_txt_start[];
extern char __w32_rsrc_start[];

extern const gfx_glyph __vid_font_8x8[];

extern uint16_t   __dospc_ds;
volatile uint32_t __dospc_counter;
dospc_isr         __dospc_bios_isr;

extern void
__dospc_pit_isr(void);

static bool _has_mouse = false;

#ifdef STACK_PROFILING
static uint64_t      *_stack_start;
static uint64_t      *_stack_end = (uint64_t *)0xFFE8;
static const uint64_t STACK_FILL_PATTERN = 0x0123456789ABCDEFULL;
#endif // STACK_PROFILING

#ifdef ZIP_PIGGYBACK
static zip_cdir_end_header *
_locate_cdir(void *from, void *to)
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
_pit_init_channel(unsigned channel, unsigned mode, unsigned divisor)
{
    if (channel > 2)
        return;
    mode &= PIT_MODE_MASK;

    _outp(PIT_IO_COMMAND, (mode << PIT_MODE) | (channel << PIT_CHANNEL) |
                              (1 << PIT_BYTE_HI) | (1 << PIT_BYTE_LO));
    _outp(PIT_IO + channel, divisor & 0xFF);
    _outp(PIT_IO + channel, divisor >> 8);
}

static void
_die_incompatible(void)
{
    char msg[TEXT_COLUMNS];
    pal_load_string(IDS_UNSUPPENV, msg, sizeof(msg));
    msg[strlen(msg)] = '$';
    dos_puts(msg);
}

static bool
_is_dos(uint8_t major)
{
    uint8_t dosMajor = dos_get_version() & 0xFF;
    return (1 == major) ? (0 == dosMajor) : (major == dosMajor);
}

static bool
_is_winnt(void)
{
    // NTVDM claims to be DOS 5
    if (!_is_dos(5))
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
_get_winnt_version(void)
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
_is_compatible(void)
{
    // We're using DOS 2.0+ API here, and we can't run on Vista and above.
    return !_is_dos(1) && (!_is_winnt() || (0x0600 > _get_winnt_version()));
}

#ifdef CONFIG_ANDREA
static bool
_pal_enum_callback(const char *name, void *data)
{
    uint16_t *drivers = (uint16_t *)data;

    for (size_t i = 0; i < MAX_DRIVERS; i++)
    {
        if (0 == drivers[i])
        {
            drivers[i] = dospc_load_driver(name);
            return true;
        }
    }

    return false;
}
#endif

static bool
_snd_enum_callback(snd_device *device, void *data)
{
    fputs("  ", stdout);
    fputs(device->name, stdout);
    for (int i = strlen(device->name); i < 8; i++)
    {
        fputc(' ', stdout);
    }
    fputs(device->description, stdout);
    fputc('\n', stdout);
    fflush(stdout);
    return true;
}

static void
_show_help(const char *self)
{
#if defined(CONFIG_ANDREA)
    uint16_t drivers[MAX_DRIVERS] = {0};
    pal_enum_assets(_pal_enum_callback, "snd*.sys", drivers);
#endif
    snd_load_inbox_drivers();

    const char *name = strrchr(self, '\\');
    if (NULL == name)
    {
        name = self;
    }
    else
    {
        name++;
    }

    fputs(name ? name : "LAVENDER", stdout);
    puts(" [/? | /S<dev>]");
    puts("\ndev:");
    snd_enum_devices(_snd_enum_callback, NULL);

#if defined(CONFIG_ANDREA)
    for (size_t i = 0; i < MAX_DRIVERS; i++)
    {
        if (0 != drivers[i])
        {
            dospc_unload_driver(drivers[i]);
        }
    }
#endif
}

void
pal_initialize(int argc, char *argv[])
{
#ifdef STACK_PROFILING
    _stack_start = (uint64_t *)((unsigned)__ebss / 8 * 8) + 1;
    for (uint64_t *ptr = _stack_start; ptr < _stack_end; ptr++)
        *ptr = STACK_FILL_PATTERN;
#endif // STACK_PROFILING

#ifdef ZIP_PIGGYBACK
    zip_cdir_end_header *cdir = _locate_cdir(__edata, __sbss);
    if (NULL == cdir)
    {
        char msg[TEXT_COLUMNS];
        pal_load_string(IDS_ERROR, msg, sizeof(msg));
        pal_load_string(IDS_NOARCHIVE, msg + strlen(msg),
                        sizeof(msg) - strlen(msg));
        pal_alert(msg, 0);
        dos_exit(1);
    }

    if (!zip_open(cdir))
#else
    if (_is_dos(2))
    {
        dos_puts("EXE \x1A DOS 3+\r\n$");
        _die_incompatible();
        dos_exit(1);
    }

    if (!zip_open(argv[0]))
#endif
    {
        char msg[TEXT_COLUMNS];
        pal_load_string(IDS_ERROR, msg, sizeof(msg));
        pal_load_string(IDS_NOARCHIVE, msg + strlen(msg),
                        sizeof(msg) - strlen(msg));
        pal_alert(msg, errno);
        dos_exit(1);
    }

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        __pal_assets[i].inzip = -1;
        __pal_assets[i].flags = 0;
        __pal_assets[i].data = NULL;
    }

#ifdef CONFIG_ANDREA
    andrea_init();
#endif

    const char *arg_snd = NULL;
    for (int i = 1; i <= argc; i++)
    {
        if ('/' != argv[i][0])
        {
            continue;
        }

        if ('s' == tolower(argv[i][1]))
        {
            arg_snd = argv[i] + 2;
        }

        if ('?' == argv[i][1])
        {
            _show_help(argv[0]);
            dos_exit(1);
        }
    }

    if (!_is_compatible())
    {
        hasset support = pal_open_asset("support.txt", O_RDONLY);
        if (NULL == support)
        {
            _die_incompatible();
            dos_exit(1);
        }

        char *data = pal_get_asset_data(support);
        if (NULL == data)
        {
            dos_exit(1);
        }

        dos_puts(data);
        bios_get_keystroke();
        dos_exit(1);
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
    _disable();
    __dospc_bios_isr = _dos_getvect(INT_PIT);
    _dos_setvect(INT_PIT, MK_FP(__libi86_get_cs(), __dospc_pit_isr));

    asm volatile("movw %%ds, %%cs:%0" : "=rm"(__dospc_ds));
    __dospc_counter = 0;
    _pit_init_channel(0, PIT_MODE_RATE_GEN, PIT_FREQ_DIVISOR);
    _enable();
#pragma GCC diagnostic pop

    if (!gfx_initialize())
    {
        pal_alert("", errno);
        _dos_setvect(INT_PIT, __dospc_bios_isr);

        _disable();
        _pit_init_channel(0, PIT_MODE_RATE_GEN, 0);
        _enable();
    }

    _has_mouse = msmouse_init();

    snd_initialize(arg_snd);
}

void
pal_cleanup(void)
{
    ziparch_cleanup();
    gfx_cleanup();
    snd_cleanup();

    _dos_setvect(INT_PIT, __dospc_bios_isr);
    _pit_init_channel(0, PIT_MODE_RATE_GEN, 0);

    nosound();

#ifdef STACK_PROFILING
    uint64_t *untouched;
    for (untouched = _stack_start; untouched < _stack_end; untouched++)
    {
        if (STACK_FILL_PATTERN != *untouched)
            break;
    }

    int stack_size = (int)(0x10000UL - (uint16_t)untouched);

    dos_puts("Stack usage: $");

    char buffer[6];
    itoa(stack_size, buffer, 10);
    for (int i = 0; buffer[i]; i++)
        dos_putc(buffer[i]);

    dos_puts("\r\n$");
#endif // STACK_PROFILING
}

bool
pal_handle(void)
{
    return true;
}

uint32_t ddcall
pal_get_counter(void)
{
    uint16_t count = 0;

    _disable();
    outp(PIT_IO_COMMAND, 0x00);
    count = inp(PIT_DATA(0));
    count |= inp(PIT_DATA(0)) << 8;
    _enable();

    return (__dospc_counter << PIT_FREQ_POWER) |
           (count & ((1 << PIT_FREQ_POWER) - 1));
}

uint32_t ddcall
pal_get_ticks(unsigned ms)
{
    uint64_t ticks = (uint64_t)ms * PIT_INPUT_FREQ;
    ticks /= 10000000ULL;
    return (UINT32_MAX < ticks) ? UINT32_MAX : ticks;
}

void ddcall
pal_sleep(unsigned ms)
{
    uint32_t until = pal_get_counter() + pal_get_ticks((uint32_t)ms);
    while (pal_get_counter() < until)
    {
        asm("hlt");
    }
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(pal_get_counter);
ANDREA_EXPORT(pal_get_ticks);
ANDREA_EXPORT(pal_sleep);
#endif

static int
_read_sector(uint8_t  drive,
             uint16_t cylinder,
             uint8_t  head,
             uint8_t  sector,
             char    *buffer)
{
    short status;
    for (int i = 0; i < 3; ++i)
    {
        status = bios_read_sectors(drive, cylinder, head, sector, 1, buffer);
        if (0 == (status & 0xFF00))
        {
            return 0;
        }

        bios_reset_disk(drive);
    }

    return -((status >> 8) & 0xFF);
}

static void
_copy_volume_label(char *dst, const char *src)
{
    memcpy(dst, src, 11);

    char *last = dst + 11;
    last[0] = ' ';
    while (' ' == *last)
        last--;
    last[1] = 0;
}

typedef struct
{
    char     label[12];
    uint32_t serial_number;
} _volume_info;

static bool
_get_volume_info(uint8_t drive, _volume_info *out)
{
    union {
        char                bytes[512];
        fat_boot_sector     boot;
        fat_directory_entry root[512 / sizeof(fat_directory_entry)];
    } sector;

    if (0 > _read_sector(drive, 0, 0, 1, sector.bytes))
    {
        return false;
    }

    int offset, size;
    switch ((uint8_t)sector.boot.Ia32Jump[0])
    {
    case 0xEB: // JMP rel8
        offset = *(int8_t *)(sector.boot.Ia32Jump + 1);
        size = offset - (sizeof(sector.boot.OemId) + 1);
        break;
    case 0xE9: // JMP rel16
        offset = *(int16_t *)(sector.boot.Ia32Jump + 1);
        size = offset - sizeof(sector.boot.OemId);
        break;
    default:
        return false;
    }

    switch (size)
    {
    case sizeof(fat_bpb20):
    case sizeof(fat_bpb30):
    case sizeof(fat_bpb32):
    case sizeof(fat_bpb33):
        out->serial_number = 0;
        out->label[0] = 0;
        break;
    case sizeof(fat_bpb34): {
        fat_bpb34 *bpb = (fat_bpb34 *)sector.boot.Payload;
        out->serial_number = bpb->Id;
        out->label[0] = 0;
        break;
    }
    case sizeof(fat_bpb40):
    case sizeof(fat_bpb71):
    case sizeof(fat_bpb71_full): {
        fat_bpb40 *bpb = (fat_bpb40 *)sector.boot.Payload;
        out->serial_number = bpb->bpb34.Id;
        _copy_volume_label(out->label, bpb->FatLabel);
        break;
    }
    default:
        return false;
    }

    fat_bpb20 *bpb = (fat_bpb20 *)sector.boot.Payload;
    uint16_t   root_entries = bpb->RootEntries;
    uint16_t   root_sectors =
        ((root_entries * sizeof(fat_directory_entry)) + (bpb->SectorSize - 1)) /
        bpb->SectorSize;
    uint16_t first_data_sector = bpb->ReservedSectors +
                                 (bpb->NoFats * bpb->SectorsPerFat) +
                                 root_sectors;
    uint16_t first_root_sector = first_data_sector - root_sectors;
    if (63 <= first_root_sector)
    {
        return false;
    }

    if (0 > _read_sector(drive, 0, 0, 1 + first_root_sector, sector.bytes))
    {
        return false;
    }

    for (int i = 0; i < (sizeof(sector) / sizeof(fat_directory_entry)); i++)
    {
        if (FAT_ATTRIBUTE_VOLUME_ID != sector.root[i].Attributes)
            continue;

        _copy_volume_label(out->label, sector.root[i].FileName);
        return true;
    }

    return false;
}

uint32_t
pal_get_medium_id(const char *tag)
{
    union {
        short          w;
        bios_equipment s;
    } equipment;
    equipment.w = bios_get_equipment_list();

    int drives = equipment.s.floppy_disk ? (equipment.s.floppy_drives + 1) : 0;
    if (2 < drives)
    {
        drives = 2;
    }

    _volume_info volume;

    uint8_t drive;
    for (drive = 0; drive < drives; drive++)
    {
        if (!_get_volume_info(drive, &volume))
            continue;

        if (0 == strcmp(volume.label, tag))
            return volume.serial_number;
    }

    errno = ENOENT;
    return 0;
}

const char *
pal_get_version_string(void)
{
    return _binary_obj_version_txt_start;
}

uint16_t
pal_get_keystroke(void)
{
    if (0 == bios_check_keystroke())
    {
        return 0;
    }

    uint16_t keystroke = bios_get_keystroke();
    if (keystroke & 0xFF)
    {
        if ('-' == (keystroke & 0xFF))
        {
            return VK_OEM_MINUS;
        }

        return toupper(keystroke & 0xFF);
    }

    switch (keystroke >> 8)
    {
    case 0x3B:
        return VK_F1;
    case 0x3C:
        return VK_F2;
    case 0x3D:
        return VK_F3;
    case 0x3E:
        return VK_F4;
    case 0x3F:
        return VK_F5;
    case 0x40:
        return VK_F6;
    case 0x41:
        return VK_F7;
    case 0x42:
        return VK_F8;
    case 0x43:
        return VK_F9;
    case 0x44:
        return VK_F10;
    case 0x47:
        return VK_HOME;
    case 0x48:
        return VK_UP;
    case 0x49:
        return VK_PRIOR;
    case 0x4B:
        return VK_LEFT;
    case 0x4D:
        return VK_RIGHT;
    case 0x4F:
        return VK_END;
    case 0x50:
        return VK_DOWN;
    case 0x51:
        return VK_NEXT;
    case 0x52:
        return VK_INSERT;
    case 0x53:
        return VK_DELETE;
    case 0x85:
        return VK_F11;
    case 0x86:
        return VK_F12;
    default:
        return 0;
    }
}

void
pal_enable_mouse(void)
{
    if (_has_mouse)
    {
        msmouse_show();
    }
}

void
pal_disable_mouse(void)
{
    if (_has_mouse)
    {
        msmouse_hide();
    }
}

uint16_t
pal_get_mouse(uint16_t *x, uint16_t *y)
{
    if (!_has_mouse)
    {
        return 0;
    }

    uint16_t lowx, lowy, status;

    status = msmouse_get_status(&lowx, &lowy);
    *x = lowx / (MSMOUSE_AREA_WIDTH / TEXT_COLUMNS);
    *y = lowy / (MSMOUSE_AREA_HEIGHT / TEXT_LINES);

    return status;
}

int
pal_load_string(unsigned id, char *buffer, int max_length)
{
    int length = exe_pe_load_string(__w32_rsrc_start, id, buffer, max_length);
    if (0 > length)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);
        return sizeof(msg) - 1;
    }

    return length;
}

void
pal_alert(const char *text, int error)
{
    char *texta = (char *)alloca(strlen(text + 1));
    utf8_encode(text, texta, pal_wctoa);

    puts("\n=====");
    puts(texta);
    if (0 != error)
    {
        char msg[80] = "errno ";
        itoa(error, msg + strlen(msg), 10);
        puts(msg);
    }

    bios_get_keystroke();
}

char
pal_wctoa(uint16_t wc)
{
    if (0x80 > wc)
    {
        return wc;
    }

    const gfx_glyph *fdata = __vid_font_8x8;
    while (wc > fdata->codepoint)
    {
        fdata++;
    }

    if (wc != fdata->codepoint)
    {
        return '?';
    }

    return fdata->base;
}

char
pal_wctob(uint16_t wc)
{
    if (0x80 > wc)
    {
        return wc;
    }

    uint8_t          local = 0x80;
    const gfx_glyph *fdata = __vid_font_8x8;

    while (wc > fdata->codepoint)
    {
        if (NULL != fdata->overlay)
        {
            local++;
        }
        fdata++;
    }

    if (wc != fdata->codepoint)
    {
        return '?';
    }

    if (NULL == fdata->overlay)
    {
        return fdata->base;
    }

    return local;
}

bool
dospc_is_dosbox(void)
{
    return 0 == _fmemcmp((const char far *)0xF000E061, "DOSBox", 6);
}

#ifdef CONFIG_ANDREA
uint16_t
dospc_load_driver(const char *name)
{
    char path[_MAX_PATH];
    if (0 > pal_extract_asset(name, path))
    {
        return 0;
    }

    andrea_module module = andrea_load(path);
    if (0 == module)
    {
        return 0;
    }

    pf_drvinit drv_init = (pf_drvinit)andrea_get_procedure(module, "drv_init");
    if (NULL == drv_init)
    {
        return 0;
    }

    if (0 > drv_init())
    {
        andrea_free(module);
        return 0;
    }

    unlink(path);
    return module;
}

void
dospc_unload_driver(uint16_t driver)
{
    pf_drvdeinit deinit =
        (pf_drvdeinit)andrea_get_procedure(driver, "drv_deinit");
    if (NULL != deinit)
    {
        deinit();
    }

    andrea_free(driver);
}
#endif // CONFIG_ANDREA

void ddcall
dospc_beep(uint16_t divisor)
{
    _pit_init_channel(2, PIT_MODE_SQUARE_WAVE_GEN, divisor);
    _outp(0x61, _inp(0x61) | SPKR_ENABLE);
}

void ddcall
dospc_silence(void)
{
    _outp(0x61, _inp(0x61) & ~SPKR_ENABLE);
}

#ifdef CONFIG_ANDREA
ANDREA_EXPORT(dospc_beep);
ANDREA_EXPORT(dospc_silence);
#endif
