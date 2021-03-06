#include <conio.h>
#include <errno.h>
#include <fcntl.h>
#include <libi86/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <api/bios.h>
#include <api/dos.h>
#include <err.h>
#include <fmt/exe.h>
#include <fmt/fat.h>
#include <fmt/zip.h>
#include <gfx.h>
#include <pal.h>
#include <platform/dospc.h>

#include "dospc.h"

typedef struct
{
    pal_timer_callback callback;
    void              *context;
} _timer_handler;

typedef struct
{
    zip_local_file_header *zip_header;
    int                    flags;
} _asset;

#define DELAY_MS_MULTIPLIER 100ULL

#define MAX_OPEN_ASSETS    8
#define MAX_TIMER_HANDLERS 2

extern char __edata[], __sbss[], __ebss[];
extern char _binary_obj_version_txt_start[];

extern const char __serrf[];
extern const char __serrm[];
extern const char StrKerError[];

static volatile uint32_t       _counter;
static dospc_isr               _bios_isr;
static volatile _timer_handler _timer_handlers[MAX_TIMER_HANDLERS];

static zip_cdir_end_header *_cdir;
static _asset               _assets[MAX_OPEN_ASSETS];

#ifdef STACK_PROFILING
static uint64_t      *_stack_start;
static uint64_t      *_stack_end = (uint64_t *)0xFFE8;
static const uint64_t STACK_FILL_PATTERN = 0x0123456789ABCDEFULL;
#endif // STACK_PROFILING

static zip_cdir_end_header *
_locate_cdir(void *from, void *to)
{
    const void *ptr = to - sizeof(zip_cdir_end_header);
    while (ptr >= from)
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

static void
_pit_init_channel(unsigned channel, unsigned mode, unsigned divisor)
{
    if (channel > 2)
        return;
    mode &= PIT_MODE_MASK;

    _disable();
    _outp(PIT_IO_COMMAND, (mode << PIT_MODE) | (channel << PIT_CHANNEL) |
                              (1 << PIT_BYTE_HI) | (1 << PIT_BYTE_LO));
    _outp(PIT_IO + channel, divisor & 0xFF);
    _outp(PIT_IO + channel, divisor >> 8);
    _enable();
}

static void far interrupt
_pit_isr(void)
{
    _disable();
    _counter++;

    if (0 == (_counter % 10))
    {
        for (int i = 0; i < MAX_TIMER_HANDLERS; i++)
        {
            if (NULL == _timer_handlers[i].callback)
            {
                continue;
            }

            _timer_handlers[i].callback(_timer_handlers[i].context);
        }
    }

    if (0 == (_counter & 0x11111))
    {
        _bios_isr();
    }
    else
    {
        _outp(PIC1_IO_COMMAND, PIC_COMMAND_EOI);
    }

    _enable();
}

// Find a message using its key byte
//   WILL CRASH IF MESSAGE NOT FOUND!
static const char *
_find_message(const char *messages, unsigned key)
{
    while (true)
    {
        if (key == *messages)
        {
            return messages + 1;
        }

        while ('$' != *messages)
        {
            messages++;
        }

        messages++;
    }
}

static void
_die_errno(void)
{
    dos_puts(StrKerError);

    char code[10];
    itoa(errno, code, 10);
    code[strlen(code)] = '$';
    dos_puts(code);
}

static void
_die_status(int error)
{
    unsigned facility = error >> 5;

    dos_puts(StrKerError);
    dos_puts(_find_message(__serrf, facility));
    dos_puts(" - $");
    dos_puts(_find_message(__serrm, error));

    dos_exit(error);
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

void
pal_initialize(void)
{
#ifdef STACK_PROFILING
    _stack_start = (uint64_t *)((unsigned)__ebss / 8 * 8) + 1;
    for (uint64_t *ptr = _stack_start; ptr < _stack_end; ptr++)
        *ptr = STACK_FILL_PATTERN;
#endif // STACK_PROFILING

    if (NULL == (_cdir = _locate_cdir(__edata, __sbss)))
    {
        _die_status(ERR_KER_ARCHIVE_NOT_FOUND);
    }

    if (!_is_compatible())
    {
        hasset support = pal_open_asset("support.txt", O_RDONLY);
        if (NULL == support)
        {
            dos_puts("Lavender cannot run in your environment.$");
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

    for (int i = 0; i < MAX_TIMER_HANDLERS; i++)
    {
        _timer_handlers[i].callback = NULL;
    }

    _disable();
    _bios_isr = _dos_getvect(INT_PIT);
    _dos_setvect(INT_PIT, _pit_isr);
    _enable();

    _counter = 0;
    _pit_init_channel(0, PIT_MODE_RATE_GEN, PIT_FREQ_DIVISOR);

    memset(_assets, 0, sizeof(_assets));

    if (!gfx_initialize())
    {
        pal_cleanup(EXIT_ERRNO);
    }
}

void
pal_cleanup(int status)
{
    gfx_cleanup();

    _dos_setvect(INT_PIT, _bios_isr);
    _pit_init_channel(0, PIT_MODE_RATE_GEN, 0);

    nosound();

    if (0 > status)
    {
        _die_status(status);
    }

    if (EXIT_ERRNO == status)
    {
        _die_errno();
    }

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

void
pal_sleep(unsigned ms)
{
    uint32_t ticks = (uint32_t)ms * DELAY_MS_MULTIPLIER;
    ticks /=
        (10000000UL * DELAY_MS_MULTIPLIER) * PIT_FREQ_DIVISOR / PIT_INPUT_FREQ;

    uint32_t until = _counter + ticks;
    while (_counter != until)
    {
        asm("hlt");
    }
}

hasset
pal_open_asset(const char *name, int flags)
{
    int slot;
    while (NULL != _assets[slot].zip_header)
    {
        slot++;

        if (MAX_OPEN_ASSETS == slot)
        {
            errno = ENOMEM;
            return NULL;
        }
    }

    if (NULL ==
        (_assets[slot].zip_header = zip_search(_cdir, name, strlen(name))))
    {
        return NULL;
    }

    _assets[slot].flags = flags;
    return (hasset)(_assets + slot);
}

bool
pal_close_asset(hasset asset)
{
    _asset *ptr = (_asset *)asset;
    if (NULL == ptr->zip_header)
    {
        errno = EBADF;
        return false;
    }

    ptr->zip_header = NULL;
    return true;
}

char *
pal_get_asset_data(hasset asset)
{
    _asset *ptr = (_asset *)asset;
    if (NULL == ptr->zip_header)
    {
        errno = EBADF;
        return NULL;
    }

    return zip_get_data(ptr->zip_header, O_RDWR == (ptr->flags & O_ACCMODE));
}

int
pal_get_asset_size(hasset asset)
{
    _asset *ptr = (_asset *)asset;
    if (NULL == ptr->zip_header)
    {
        errno = EBADF;
        return -1;
    }

    return ptr->zip_header->compressed_size;
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

    if (0 != dos_read_disk(drive, 1, 0, sector.bytes))
    {
        return false;
    }

    if (0xAA55U != sector.boot.Magic)
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
    case sizeof(fat_bpb40): {
        fat_bpb40 *bpb = (fat_bpb40 *)sector.boot.Payload;
        out->serial_number = bpb->bpb34.Id;
        _copy_volume_label(out->label, bpb->FatLabel);
        break;
    }
    case sizeof(fat_bpb71): {
        fat_bpb71 *bpb = (fat_bpb71 *)sector.boot.Payload;
        out->serial_number = bpb->Id;
        out->label[0] = 0;
        break;
    }
    case sizeof(fat_bpb71_full): {
        fat_bpb71_full *bpb = (fat_bpb71_full *)sector.boot.Payload;
        out->serial_number = bpb->bpb71.Id;
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

    if (0 != dos_read_disk(drive, 1, first_root_sector, sector.bytes))
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
    bios_equipment equipment;
    *(short *)&equipment = bios_get_equipment_list();

    int drives = equipment.floppy_disk ? (equipment.floppy_drives + 1) : 0;
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

htimer
pal_register_timer_callback(pal_timer_callback callback, void *context)
{
    int i;

    _disable();
    for (i = 0; i < MAX_TIMER_HANDLERS; i++)
    {
        if (NULL != _timer_handlers[i].callback)
        {
            continue;
        }

        _timer_handlers[i].callback = callback;
        _timer_handlers[i].context = context;
        break;
    }
    _enable();

    if (MAX_TIMER_HANDLERS == i)
    {
        errno = ENOMEM;
        i = -1;
    }

    return (htimer)i;
}

bool
pal_unregister_timer_callback(htimer timer)
{
    int i = (int)timer;
    if (MAX_TIMER_HANDLERS <= i)
    {
        errno = ENOENT;
        return false;
    }

    _disable();
    _timer_handlers[i].callback = NULL;
    _enable();
}

bool
dospc_is_dosbox(void)
{
    return 0 == _fmemcmp((const char far *)0xF000E061, "DOSBox", 6);
}

void
dospc_beep(uint16_t divisor)
{
    _pit_init_channel(2, PIT_MODE_SQUARE_WAVE_GEN, divisor);
    _outp(0x61, _inp(0x61) | SPKR_ENABLE);
}

void
dospc_silence(void)
{
    nosound();
}
