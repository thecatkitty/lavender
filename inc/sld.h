#ifndef _SLD_H_
#define _SLD_H_

#ifndef __ASSEMBLER__

#include <gfx.h>

#define SLD_ENTRY_MAX_LENGTH 255

#define SLD_TAG_PREFIX_LABEL ':'
#define SLD_TAG_TYPE_TEXT    'T'
#define SLD_TAG_TYPE_BITMAP  'B'
#define SLD_TAG_TYPE_RECT    'R'
#define SLD_TAG_TYPE_RECTF   'r'
#define SLD_TAG_TYPE_PLAY    'P'
#define SLD_TAG_TYPE_WAITKEY 'K'
#define SLD_TAG_TYPE_JUMP    'J'
#define SLD_TAG_TYPE_JUMPE   '='
#define SLD_TAG_TYPE_CALL    '!'
#define SLD_TAG_ALIGN_LEFT   '<'
#define SLD_TAG_ALIGN_CENTER '^'
#define SLD_TAG_ALIGN_RIGHT  '>'

typedef enum
{
    SLD_TYPE_BLANK,
    SLD_TYPE_LABEL,
    SLD_TYPE_TEXT,
    SLD_TYPE_BITMAP,
    SLD_TYPE_RECT,
    SLD_TYPE_RECTF,
    SLD_TYPE_PLAY,
    SLD_TYPE_WAITKEY,
    SLD_TYPE_JUMP,
    SLD_TYPE_JUMPE,
    SLD_TYPE_CALL
} sld_type;

#define SLD_ALIGN_LEFT   0
#define SLD_ALIGN_CENTER 0xFFF1
#define SLD_ALIGN_RIGHT  0xFFF2

#define SLD_METHOD_STORE 0
#define SLD_METHOD_XOR48 1

#define SLD_PARAMETER_XOR48_INLINE 0
#define SLD_PARAMETER_XOR48_PROMPT 1
#define SLD_PARAMETER_XOR48_SPLIT  2
#define SLD_PARAMETER_XOR48_DISKID 3

typedef struct
{
    uint16_t delay;
    uint16_t posx;
    uint16_t posy;
    uint8_t  type;
    union {
        uint8_t content[SLD_ENTRY_MAX_LENGTH + 1];
        struct
        {
            gfx_dimensions dimensions;
            gfx_color      color;
        } shape;
        struct
        {
            char     file_name[64];
            uint16_t method;
            uint32_t crc32;
            uint16_t parameter;
            char     data[128];
        } script_call;
    };
    uint8_t length;
} sld_entry;

// Execute a script
// Returns negative on error
extern int
sld_run_script(void *script, int size);

// Load the next line from the script
// Returns total line length in bytes, negative on error
extern int
sld_load_entry(const char *line, sld_entry *out);

#endif // __ASSEMBLER__

#include <err.h>

#define ERR_SLD_INVALID_DELAY      ERR_CODE(ERR_FACILITY_SLD, 0)
#define ERR_SLD_UNKNOWN_TYPE       ERR_CODE(ERR_FACILITY_SLD, 1)
#define ERR_SLD_INVALID_VERTICAL   ERR_CODE(ERR_FACILITY_SLD, 2)
#define ERR_SLD_INVALID_HORIZONTAL ERR_CODE(ERR_FACILITY_SLD, 3)
#define ERR_SLD_CONTENT_TOO_LONG   ERR_CODE(ERR_FACILITY_SLD, 4)
#define ERR_SLD_LABEL_NOT_FOUND    ERR_CODE(ERR_FACILITY_SLD, 5)
#define ERR_SLD_INVALID_COMPARISON ERR_CODE(ERR_FACILITY_SLD, 6)

#endif // _SLD_H_
