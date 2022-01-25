#ifndef _KER_H_
#define _KER_H_

#ifndef __ASSEMBLER__

#include <base.h>
#include <fmt/zip.h>

#define interrupt __attribute__((interrupt))

typedef void(interrupt *isr)(void) far;

extern bool
KerIsDosBox(void);

extern bool
KerIsDosMajor(uint8_t major);

extern unsigned
KerGetTicksFromMs(unsigned ms);

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

// Convert string from UTF-8 to another encoding
// Returns the length of the converted string, negative on error
extern int
KerConvertFromUtf8(const char *src, char *dst, char (*encoder)(uint16_t));

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

// Start playing music
extern void
KerStartPlayer(void *music, uint16_t length);

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_KER_UNSUPPORTED       ERR_CODE(ERR_FACILITY_KER, 1)
#define ERR_KER_NOT_FOUND         ERR_CODE(ERR_FACILITY_KER, 2)
#define ERR_KER_ARCHIVE_NOT_FOUND ERR_CODE(ERR_FACILITY_KER, 3)
#define ERR_KER_ARCHIVE_TOO_LARGE ERR_CODE(ERR_FACILITY_KER, 4)
#define ERR_KER_ARCHIVE_INVALID   ERR_CODE(ERR_FACILITY_KER, 5)
#define ERR_KER_INVALID_SEQUENCE  ERR_CODE(ERR_FACILITY_KER, 6)

#endif // _KER_H_
