#ifndef _SLD_H_
#define _SLD_H_

#ifndef __ASSEMBLER__

#include <stdint.h>

#include <ker.h>

#define SLD_ENTRY_MAX_LENGTH 80

#define SLD_TAG_TYPE_TEXT    'T'
#define SLD_TAG_TYPE_BITMAP  'B'
#define SLD_TAG_ALIGN_LEFT   '<'
#define SLD_TAG_ALIGN_CENTER '^'
#define SLD_TAG_ALIGN_RIGHT  '>'

#define SLD_TYPE_TEXT   0
#define SLD_TYPE_BITMAP 1

#define SLD_ALIGN_LEFT   0
#define SLD_ALIGN_CENTER 0xFFF1
#define SLD_ALIGN_RIGHT  0xFFF2

typedef struct
{
    uint16_t Delay;
    uint16_t Horizontal;
    uint16_t Vertical;
    uint8_t  Type;
    uint8_t  Content[SLD_ENTRY_MAX_LENGTH + 1];
    uint8_t  Length;
} SLD_ENTRY;

// Load the next line from the slideshow file
// Returns total line length in bytes, negative on error
extern int
SldLoadEntry(const char *line, SLD_ENTRY *out);

// Execute a line loaded from the slideshow file
// Returns negative on error
extern int
SldExecuteEntry(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip);

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_SLD_INVALID_DELAY      ERR_CODE(ERR_FACILITY_SLD, 0)
#define ERR_SLD_UNKNOWN_TYPE       ERR_CODE(ERR_FACILITY_SLD, 1)
#define ERR_SLD_INVALID_VERTICAL   ERR_CODE(ERR_FACILITY_SLD, 2)
#define ERR_SLD_INVALID_HORIZONTAL ERR_CODE(ERR_FACILITY_SLD, 3)
#define ERR_SLD_CONTENT_TOO_LONG   ERR_CODE(ERR_FACILITY_SLD, 4)

#endif // _SLD_H_
