#include <conio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libi86/string.h>

#include <api/bios.h>
#include <api/dos.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <fmt/exe.h>
#include <fmt/zip.h>
#include <ker.h>
#include <pal.h>
#include <pal/dospc.h>

typedef struct
{
    ZIP_LOCAL_FILE_HEADER *zip_header;
    int                    flags;
} _asset;

#define SPKR_ENABLE         3
#define PIT_INPUT_FREQ      11931816667ULL
#define PIT_FREQ_DIVISOR    2048ULL
#define DELAY_MS_MULTIPLIER 100ULL

#define MAX_OPEN_ASSETS 8

extern char __edata[], __sbss[], __ebss[];
extern char _binary_obj_version_txt_start[];

extern const char __serrf[];
extern const char __serrm[];
extern const char StrKerError[];

static volatile uint32_t _counter;
static ISR               _bios_isr;

static ZIP_CDIR_END_HEADER *_cdir;
static _asset               _assets[MAX_OPEN_ASSETS];

#ifdef STACK_PROFILING
static uint64_t      *_stack_start;
static uint64_t      *_stack_end = (uint64_t *)0xFFE8;
static const uint64_t STACK_FILL_PATTERN = 0x0123456789ABCDEFULL;
#endif // STACK_PROFILING

static ZIP_CDIR_END_HEADER *
_locate_cdir(void *from, void *to)
{
    const void *ptr = to - sizeof(ZIP_CDIR_END_HEADER);
    while (ptr >= from)
    {
        ZIP_CDIR_END_HEADER *cdir = (ZIP_CDIR_END_HEADER *)ptr;
        if ((ZIP_PK_SIGN == cdir->PkSignature) &&
            (ZIP_CDIR_END_SIGN == cdir->HeaderSignature))
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
    DosPutS(StrKerError);

    char code[10];
    itoa(errno, code, 10);
    code[strlen(code)] = '$';
    DosPutS(code);
}

static void
_die_status(int error)
{
    unsigned facility = error >> 5;

    DosPutS(StrKerError);
    DosPutS(_find_message(__serrf, facility));
    DosPutS(" - $");
    DosPutS(_find_message(__serrm, error));

    DosExit(error);
}

static bool
_is_dos(uint8_t major)
{
    uint8_t dosMajor = DosGetVersion() & 0xFF;
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
        EXE_DOS_HEADER         DosHdr;
        ULONG                  NewSignature;
        EXE_PE_OPTIONAL_HEADER OptionalHeader;
    } data;

    read(fd, &data, sizeof(EXE_DOS_HEADER));
    if (0x5A4D != data.DosHdr.e_magic)
    {
        return 0; // Invalid executable
    }

    lseek(fd, data.DosHdr.e_lfanew, SEEK_SET);
    read(fd, &data, sizeof(ULONG));
    if (0x00004550 != data.NewSignature)
    {
        return 0; // Invalid executable
    }

    lseek(fd, sizeof(EXE_PE_FILE_HEADER), SEEK_CUR);
    read(fd, &data, sizeof(EXE_PE_OPTIONAL_HEADER));
    close(fd);

    return (data.OptionalHeader.MajorImageVersion << 8) |
           data.OptionalHeader.MinorImageVersion;
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
            DosPutS("Lavender cannot run in your environment.$");
            DosExit(1);
        }

        char *data = pal_get_asset_data(support);
        if (NULL == data)
        {
            DosExit(1);
        }

        DosPutS(data);
        BiosKeyboardGetKeystroke();
        DosExit(1);
    }

    _disable();
    _bios_isr = _dos_getvect(INT_PIT);
    _dos_setvect(INT_PIT, _pit_isr);
    _enable();

    _counter = 0;
    _pit_init_channel(0, PIT_MODE_RATE_GEN, PIT_FREQ_DIVISOR);

    memset(_assets, 0, sizeof(_assets));
}

void
pal_cleanup(int status)
{
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

    DosPutS("Stack usage: $");

    char buffer[6];
    itoa(stack_size, buffer, 10);
    for (int i = 0; buffer[i]; i++)
        DosPutC(buffer[i]);

    DosPutS("\r\n$");
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

void
pal_beep(uint16_t divisor)
{
    _pit_init_channel(2, PIT_MODE_SQUARE_WAVE_GEN, divisor);
    _outp(0x61, _inp(0x61) | SPKR_ENABLE);
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

    return ptr->zip_header->CompressedSize;
}

const char *
pal_get_version_string(void)
{
    return _binary_obj_version_txt_start;
}

bool
dospc_is_dosbox(void)
{
    return 0 == _fmemcmp((const char far *)0xF000E061, "DOSBox", 6);
}
