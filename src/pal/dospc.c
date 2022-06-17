#include <conio.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <api/dos.h>
#include <dev/pic.h>
#include <dev/pit.h>
#include <fmt/zip.h>
#include <ker.h>
#include <pal.h>

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
const char *
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
