#ifndef _SLD_H_
#define _SLD_H_

#include <pal.h>

#define SLD_VIEWBOX_WIDTH  640
#define SLD_VIEWBOX_HEIGHT 200

#define SLD_ENTRY_MAX_LENGTH 255

#define SLD_TAG_PREFIX_LABEL ':'
#define SLD_TAG_TYPE_TEXT    'T'
#define SLD_TAG_TYPE_BITMAP  'B'
#define SLD_TAG_TYPE_RECT    'R'
#define SLD_TAG_TYPE_RECTF   'r'
#define SLD_TAG_TYPE_PLAY    'P'
#define SLD_TAG_TYPE_WAITKEY 'K'
#define SLD_TAG_TYPE_ACTAREA 'A'
#define SLD_TAG_TYPE_QUERY   '?'
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
    SLD_TYPE_ACTAREA,
    SLD_TYPE_QUERY,
    SLD_TYPE_JUMP,
    SLD_TYPE_JUMPE,
    SLD_TYPE_CALL
} sld_type;

typedef enum
{
    SLD_STATE_LOAD,
    SLD_STATE_EXECUTE,
    SLD_STATE_DELAY,
    SLD_STATE_WAIT,
    SLD_STATE_STOP,
    SLD_SYSERR = -1,
    SLD_ARGERR = -2,
    SLD_QUIT = -3
} sld_state;

#define SLD_ALIGN_LEFT   0
#define SLD_ALIGN_CENTER 0xFFF1
#define SLD_ALIGN_RIGHT  0xFFF2

#define SLD_METHOD_STORE 0
#define SLD_METHOD_XOR48 1
#define SLD_METHOD_DES   2

#define SLD_KEYSOURCE_INLINE 0
#define SLD_KEYSOURCE_PROMPT 1

#define SLD_PARAMETER_XOR48_INLINE SLD_KEYSOURCE_INLINE
#define SLD_PARAMETER_XOR48_PROMPT SLD_KEYSOURCE_PROMPT
#define SLD_PARAMETER_XOR48_SPLIT  8
#define SLD_PARAMETER_XOR48_DISKID 9

#define SLD_PARAMETER_DES_INLINE SLD_KEYSOURCE_INLINE
#define SLD_PARAMETER_DES_PROMPT SLD_KEYSOURCE_PROMPT
#define SLD_PARAMETER_DES_PKEY   8

typedef struct
{
    uint16_t delay;
    uint16_t posx;
    uint16_t posy;
    uint8_t  type;
    char     content[SLD_ENTRY_MAX_LENGTH + 1];
    uint8_t  length;
} sld_entry;

typedef struct _sld_context
{
    hasset    script;
    void     *data;
    int       size;
    int       offset;
    sld_state state;
    uint32_t  next_step;
    union {
        sld_entry entry;
        char      message[sizeof(sld_entry)];
    };
    struct _sld_context *parent;
} sld_context;

extern sld_context *
sld_create_context(const char *name, sld_context *parent);

extern bool
sld_close_context(sld_context *ctx);

extern void
sld_enter_context(sld_context *ctx);

extern sld_context *
sld_exit_context(void);

// Start script execution
extern void
sld_run(sld_context *ctx);

// Handle a next step of the script execution
extern void
sld_handle(void);

// Load the next line from the script
// Returns total line length in bytes, negative on error
extern int
sld_load_entry(sld_context *ctx, sld_entry *entry);

#endif // _SLD_H_
