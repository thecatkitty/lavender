#ifndef _KER_H_
#define _KER_H_

#include <stdbool.h>

#include <err.h>
#include <fmt/zip.h>

#ifndef EDITING
#define far __far
#else
#define far
#endif

#define interrupt __attribute__((interrupt))

typedef void(interrupt *isr)(void) far;

#define KER_PIT_INPUT_FREQ      11931816667
#define KER_PIT_FREQ_DIVISOR    2048
#define KER_DELAY_MS_MULTIPLIER 100
#define KER_DELAY_MS_DIVISOR                                                   \
    ((10000000 * KER_DELAY_MS_MULTIPLIER) * KER_PIT_FREQ_DIVISOR /             \
     KER_PIT_INPUT_FREQ)

#define ERR_KER_UNSUPPORTED       ERR_CODE(ERR_FACILITY_KER, 1)
#define ERR_KER_NOT_FOUND         ERR_CODE(ERR_FACILITY_KER, 2)
#define ERR_KER_ARCHIVE_NOT_FOUND ERR_CODE(ERR_FACILITY_KER, 3)
#define ERR_KER_ARCHIVE_TOO_LARGE ERR_CODE(ERR_FACILITY_KER, 4)
#define ERR_KER_ARCHIVE_INVALID   ERR_CODE(ERR_FACILITY_KER, 5)
#define ERR_KER_INVALID_SEQUENCE  ERR_CODE(ERR_FACILITY_KER, 6)

extern bool
KerIsDosBox(void);

extern void
KerSleep(unsigned ticks);

extern isr
KerInstallIsr(isr routine, unsigned number);

extern void
KerUninstallIsr(isr previous, unsigned number);

inline void
KerDisableInterrupts()
{
    asm("cli");
}

inline void
KerEnableInterrupts()
{
    asm("sti");
}

// Get code point from UTF-8 sequence
// Returns the length of the sequence (0 when NUL), negative on error
extern int
KerGetCharacterFromUtf8(const char *sequence, uint16_t *codePoint);

// Conduct a case-insensitive comparison of two UTF-8 strings
// Returns 0 when equal, 1 when different, negative on error
extern int
KerCompareUtf8IgnoreCase(const char *str1, const char *str2, unsigned length);

// Locate ZIP central directory end structure
// Returns 0 when found, negative on error
extern int
KerLocateArchive(void *from, void *to, ZIP_CDIR_END_HEADER **cdir);

// Locate ZIP local file header structure
// Returns 0 when found, negative on error
extern int
KerSearchArchive(ZIP_CDIR_END_HEADER *   cdir,
                 const char *            name,
                 uint16_t                nameLength,
                 ZIP_LOCAL_FILE_HEADER **lfh);

// Locate ZIP file data
// Returns 0 when found, negative on error
extern int
KerGetArchiveData(ZIP_LOCAL_FILE_HEADER *lfh, void **data);

#endif // _KER_H_
