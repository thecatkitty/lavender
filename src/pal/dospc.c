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
#include <api/msmouse.h>
#include <fmt/exe.h>
#include <fmt/fat.h>
#include <fmt/utf8.h>
#include <fmt/zip.h>
#include <gfx.h>
#include <pal.h>
#include <platform/dospc.h>

#include "../resource.h"
#include "dospc.h"

typedef struct
{
    off_t inzip;
    int   flags;
    char *data;
} _asset;

#define DELAY_MS_MULTIPLIER 100ULL

#define MAX_OPEN_ASSETS 8

#define TEXT_COLUMNS 80
#define TEXT_LINES   25

extern char __edata[], __sbss[], __ebss[];
extern char _binary_obj_version_txt_start[];
extern char __w32_rsrc_start[];

extern uint16_t   __dospc_ds;
volatile uint32_t __dospc_counter;
dospc_isr         __dospc_bios_isr;

extern void
__dospc_pit_isr(void);

extern void
__snd_timer_callback(void);

static _asset _assets[MAX_OPEN_ASSETS];
static bool   _has_mouse = false;

#ifdef STACK_PROFILING
static uint64_t      *_stack_start;
static uint64_t      *_stack_end = (uint64_t *)0xFFE8;
static const uint64_t STACK_FILL_PATTERN = 0x0123456789ABCDEFULL;
#endif // STACK_PROFILING

#ifdef ZIP_PIGGYBACK
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
_die_errno(void)
{
    char msg[TEXT_COLUMNS] = "errno ";
    itoa(errno, msg + strlen(msg), 10);
    msg[strlen(msg)] = '$';
    dos_puts(msg);
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
        msg[strlen(msg)] = '$';
        dos_puts(msg);

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
        msg[strlen(msg)] = '$';
        dos_puts(msg);

        dos_puts("\r\n$");
        _die_errno();
        dos_exit(1);
    }

    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        _assets[i].inzip = -1;
        _assets[i].flags = 0;
        _assets[i].data = NULL;
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

    _disable();
    __dospc_bios_isr = _dos_getvect(INT_PIT);
    _dos_setvect(INT_PIT, MK_FP(__libi86_get_cs(), __dospc_pit_isr));

    asm volatile("movw %%ds, %%cs:%0" : "=rm"(__dospc_ds));
    __dospc_counter = 0;
    _pit_init_channel(0, PIT_MODE_RATE_GEN, PIT_FREQ_DIVISOR);
    _enable();

    if (!gfx_initialize())
    {
        _die_errno();
        _dos_setvect(INT_PIT, __dospc_bios_isr);

        _disable();
        _pit_init_channel(0, PIT_MODE_RATE_GEN, 0);
        _enable();
    }

    _has_mouse = msmouse_init();
}

void
pal_cleanup(void)
{
    for (int i = 0; i < MAX_OPEN_ASSETS; ++i)
    {
        if (NULL != _assets[i].data)
        {
            zip_free_data(_assets[i].data);
        }
    }

    gfx_cleanup();

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

void
pal_sleep(unsigned ms)
{
    uint32_t ticks = (uint32_t)ms * DELAY_MS_MULTIPLIER;
    ticks /=
        (10000000UL * DELAY_MS_MULTIPLIER) * PIT_FREQ_DIVISOR / PIT_INPUT_FREQ;

    uint32_t until = __dospc_counter + ticks;
    while (__dospc_counter < until)
    {
        asm("hlt");
    }
}

hasset
pal_open_asset(const char *name, int flags)
{
    off_t lfh = zip_search(name, strlen(name));
    if (-1 == lfh)
    {
        return NULL;
    }

    int slot = 0;
    while (-1 != _assets[slot].inzip)
    {
        if (lfh == _assets[slot].inzip)
        {
            if (O_RDWR == (flags & O_ACCMODE))
            {
                _assets[slot].flags = (flags & ~O_ACCMODE) | O_RDWR;
            }

            return (hasset)(_assets + slot);
        }

        slot++;
        if (MAX_OPEN_ASSETS == slot)
        {
            errno = ENOMEM;
            return NULL;
        }
    }

    _assets[slot].inzip = lfh;
    _assets[slot].flags = flags;
    return (hasset)(_assets + slot);
}

bool
pal_close_asset(hasset asset)
{
    _asset *ptr = (_asset *)asset;
    if (-1 == ptr->inzip)
    {
        errno = EBADF;
        return false;
    }

    if (O_RDWR == (ptr->flags & O_ACCMODE))
    {
        // Do not release modified assets
        return true;
    }

    ptr->inzip = -1;

    if (NULL != ptr->data)
    {
        zip_free_data(ptr->data);
        ptr->data = NULL;
    }
    return true;
}

char *
pal_get_asset_data(hasset asset)
{
    _asset *ptr = (_asset *)asset;
    if (-1 == ptr->inzip)
    {
        errno = EBADF;
        return NULL;
    }

    if (NULL == ptr->data)
    {
        ptr->data = zip_get_data(ptr->inzip);
    }

    return ptr->data;
}

int
pal_get_asset_size(hasset asset)
{
    _asset *ptr = (_asset *)asset;
    if (-1 == ptr->inzip)
    {
        errno = EBADF;
        return -1;
    }

    return zip_get_size(ptr->inzip);
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

    return bios_get_keystroke() >> 8;
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
    exe_pe_resource_directory       *dir;
    exe_pe_resource_directory_entry *ent;
    exe_pe_resource_data_entry      *data;

    // Type: RT_STRING
    dir = (exe_pe_resource_directory *)__w32_rsrc_start;
    ent = (exe_pe_resource_directory_entry *)(dir + 1);
    ent += dir->NumberOfNamedEntries;
    for (int i = 0;
         (dir->NumberOfIdEntries > i) && (EXE_PE_RT_STRING != ent->Id);
         ++i, ++ent)
        ;

    if (EXE_PE_RT_STRING != ent->Id)
    {
        const char msg[] = "!!! RT_STRING missing !!!";
        strncpy(buffer, msg, max_length);
        return sizeof(msg) - 1;
    }

    // Item: string ID / 16 + 1
    dir = (exe_pe_resource_directory *)(__w32_rsrc_start +
                                        ent->dir.OffsetToDirectory);
    ent = (exe_pe_resource_directory_entry *)(dir + 1);
    ent += dir->NumberOfNamedEntries;
    for (int i = 0;
         (dir->NumberOfIdEntries > i) && (((id >> 4) + 1) != ent->Id);
         ++i, ++ent)
        ;

    if (((id >> 4) + 1) != ent->Id)
    {
        const char msg[] = "!!! string missing !!!";
        strncpy(buffer, msg, max_length);
        return sizeof(msg) - 1;
    }

    // Language: first available
    dir = (exe_pe_resource_directory *)(__w32_rsrc_start +
                                        ent->dir.OffsetToDirectory);
    ent = (exe_pe_resource_directory_entry *)(dir + 1);
    data = (exe_pe_resource_data_entry *)(__w32_rsrc_start + ent->OffsetToData);

    // One table = 16 strings
    uint16_t *wstr = (uint16_t *)((WORD)data->OffsetToData);
    for (int i = 0; (id & 0xF) > i; ++i)
    {
        wstr += *wstr + 1;
    }

    // First WORD is string length
    uint16_t *end = wstr + *wstr + 1;
    wstr++;

    // Convert WSTR to UTF-8
    char *buffptr = buffer;
    while ((wstr < end) && (buffer + max_length > buffptr))
    {
        char mb[3];

        int seqlen = utf8_get_sequence(*wstr, mb);
        if (buffer + max_length < buffptr + seqlen)
        {
            break;
        }

        memcpy(buffptr, mb, seqlen);
        wstr++;
        buffptr += seqlen;
    }

    *buffptr = 0;
    return buffptr - buffer;
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
    _outp(0x61, _inp(0x61) & ~SPKR_ENABLE);
}
