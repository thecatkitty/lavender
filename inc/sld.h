#ifndef _SLD_H_
#define _SLD_H_

#ifndef __ASSEMBLER__

#include <gfx.h>
#include <ker.h>

#define SLD_ENTRY_MAX_LENGTH 255

#define SLD_TAG_PREFIX_LABEL ':'
#define SLD_TAG_TYPE_TEXT    'T'
#define SLD_TAG_TYPE_BITMAP  'B'
#define SLD_TAG_TYPE_RECT    'R'
#define SLD_TAG_TYPE_RECTF   'r'
#define SLD_TAG_TYPE_PLAY    'P'
#define SLD_TAG_TYPE_JUMP    'J'
#define SLD_TAG_TYPE_WAITKEY 'K'
#define SLD_TAG_TYPE_KEYJUMP 'j'
#define SLD_TAG_ALIGN_LEFT   '<'
#define SLD_TAG_ALIGN_CENTER '^'
#define SLD_TAG_ALIGN_RIGHT  '>'

typedef enum
{
    SLD_TYPE_LABEL,
    SLD_TYPE_TEXT,
    SLD_TYPE_BITMAP,
    SLD_TYPE_RECT,
    SLD_TYPE_RECTF,
    SLD_TYPE_PLAY,
    SLD_TYPE_JUMP,
    SLD_TYPE_WAITKEY,
    SLD_TYPE_KEYJUMP
} SLD_TYPE;

#define SLD_ALIGN_LEFT   0
#define SLD_ALIGN_CENTER 0xFFF1
#define SLD_ALIGN_RIGHT  0xFFF2

typedef struct
{
    uint16_t Delay;
    uint16_t Horizontal;
    uint16_t Vertical;
    uint8_t  Type;
    union {
        uint8_t Content[SLD_ENTRY_MAX_LENGTH + 1];
        struct
        {
            GFX_DIMENSIONS Dimensions;
            GFX_COLOR      Color;
        } Shape;
    };
    uint8_t Length;
} SLD_ENTRY;

// Load the next line from the slideshow file
// Returns total line length in bytes, negative on error
extern int
SldLoadEntry(const char *line, SLD_ENTRY *out);

// Execute a line loaded from the slideshow file
// Returns negative on error
extern int
SldExecuteEntry(SLD_ENTRY *sld, ZIP_CDIR_END_HEADER *zip);

// Find the first line after the given label
// Returns negative on error
extern int
SldFindLabel(const char *start, const char *label, const char **line);

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_SLD_INVALID_DELAY      ERR_CODE(ERR_FACILITY_SLD, 0)
#define ERR_SLD_UNKNOWN_TYPE       ERR_CODE(ERR_FACILITY_SLD, 1)
#define ERR_SLD_INVALID_VERTICAL   ERR_CODE(ERR_FACILITY_SLD, 2)
#define ERR_SLD_INVALID_HORIZONTAL ERR_CODE(ERR_FACILITY_SLD, 3)
#define ERR_SLD_CONTENT_TOO_LONG   ERR_CODE(ERR_FACILITY_SLD, 4)
#define ERR_SLD_LABEL_NOT_FOUND    ERR_CODE(ERR_FACILITY_SLD, 5)
#define ERR_SLD_INVALID_KEYCODE    ERR_CODE(ERR_FACILITY_SLD, 6)

#endif // _SLD_H_
