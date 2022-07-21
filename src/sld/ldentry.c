#include <ctype.h>
#include <stdlib.h>

#include <api/dos.h>
#include <fmt/utf8.h>
#include <gfx.h>
#include <sld.h>

static int
SldLoadPosition(const char *str, sld_entry *out);

static int
SldLoadContent(const char *str, sld_entry *out);

static int
SldConvertText(const char *, sld_entry *inOut);

static int
SldLoadConditional(const char *str, sld_entry *out);

static int
SldLoadShape(const char *str, sld_entry *out);

static int
SldLoadScriptCall(const char *str, sld_entry *out);

static int
SldLoadU(const char *str, uint16_t *out);

#define PROCESS_LOAD_PIPELINE(stage, str, out)                                 \
    {                                                                          \
        int length;                                                            \
        if (0 > (length = stage(str, out)))                                    \
        {                                                                      \
            return length;                                                     \
        };                                                                     \
        str += length;                                                         \
    }

int
sld_load_entry(const char *line, sld_entry *out)
{
    const char *cur = line;
    if ('\r' == *cur)
    {
        out->type = SLD_TYPE_BLANK;
        return ('\n' == cur[1]) ? 2 : 1;
    }

    int      length;
    uint16_t num;

    if (SLD_TAG_PREFIX_LABEL == *cur)
    {
        out->type = SLD_TYPE_LABEL;
        out->delay = 0;
        cur++;
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        goto End;
    }

    // Load delay
    length = SldLoadU(cur, &num);
    if (0 > length)
    {
        ERR(SLD_INVALID_DELAY);
    }
    out->delay = num;
    cur += length;

    // Load type
    uint8_t typeTag = *cur;
    cur++;

    // Process all parts
    switch (typeTag)
    {
    case SLD_TAG_TYPE_TEXT:
        out->type = SLD_TYPE_TEXT;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        PROCESS_LOAD_PIPELINE(SldConvertText, cur, out);
        break;
    case SLD_TAG_TYPE_BITMAP:
        out->type = SLD_TYPE_BITMAP;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        break;
    case SLD_TAG_TYPE_RECT:
        out->type = SLD_TYPE_RECT;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadShape, cur, out);
        break;
    case SLD_TAG_TYPE_RECTF:
        out->type = SLD_TYPE_RECTF;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadShape, cur, out);
        break;
    case SLD_TAG_TYPE_PLAY:
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        out->type = SLD_TYPE_PLAY;
        break;
    case SLD_TAG_TYPE_WAITKEY:
        out->type = SLD_TYPE_WAITKEY;
        break;
    case SLD_TAG_TYPE_JUMP:
        out->type = SLD_TYPE_JUMP;
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        break;
    case SLD_TAG_TYPE_JUMPE:
        out->type = SLD_TYPE_JUMPE;
        PROCESS_LOAD_PIPELINE(SldLoadConditional, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        break;
    case SLD_TAG_TYPE_CALL:
        out->type = SLD_TYPE_CALL;
        PROCESS_LOAD_PIPELINE(SldLoadScriptCall, cur, out);
        break;
    default:
        ERR(SLD_UNKNOWN_TYPE);
    }

End:
    // Ignore rest of the line
    while (('\r' != *cur) && ('\n' != *cur))
    {
        cur++;
    }

    while (('\r' == *cur) || ('\n' == *cur))
    {
        cur++;
    }

    return cur - line;
}

int
SldLoadPosition(const char *str, sld_entry *out)
{
    const char *cur = str;
    int         length;

    // Load vertical position
    length = SldLoadU(cur, &out->posy);
    if (0 > length)
    {
        ERR(SLD_INVALID_VERTICAL);
    }
    cur += length;

    // Load horizontal position
    length = SldLoadU(cur, &out->posx);
    if (0 <= length)
    {
        cur += length;
    }
    else
    {
        switch (*cur)
        {
        case SLD_TAG_ALIGN_LEFT:
            out->posx = SLD_ALIGN_LEFT;
            break;
        case SLD_TAG_ALIGN_RIGHT:
            out->posx = SLD_ALIGN_RIGHT;
            break;
        case SLD_TAG_ALIGN_CENTER:
            out->posx = SLD_ALIGN_CENTER;
            break;
        default:
            ERR(SLD_INVALID_HORIZONTAL);
        }
        cur++;
    }

    return cur - str;
}

int
SldLoadContent(const char *str, sld_entry *out)
{
    const char *cur = str;
    char       *content = out->content;
    int         length = 0;

    while (isspace(*cur))
    {
        cur++;
    }

    while (('\r' != *cur) && ('\n' != *cur))
    {
        if (SLD_ENTRY_MAX_LENGTH < length)
        {
            ERR(SLD_CONTENT_TOO_LONG);
        }
        *(content++) = *(cur++);
        length++;
    }
    *content = 0;

    out->length = length;
    return cur - str;
}

int
SldConvertText(const char *str, sld_entry *inOut)
{
    inOut->length = utf8_encode(inOut->content, inOut->content, gfx_wctob);
    if (0 > inOut->length)
    {
        ERR(KER_INVALID_SEQUENCE);
    }

    return 0;
}

int
SldLoadConditional(const char *str, sld_entry *out)
{
    const char *cur = str;
    int         length = 0;

    length = SldLoadU(cur, &out->posy);
    if (0 > length)
    {
        ERR(SLD_INVALID_COMPARISON);
    }
    cur += length;

    return cur - str;
}

int
SldLoadShape(const char *str, sld_entry *out)
{
    const char *cur = str;
    uint16_t    width, height;

    cur += SldLoadU(cur, &width);
    cur += SldLoadU(cur, &height);

    switch (*cur)
    {
    case 'B':
        out->shape.color = GFX_COLOR_BLACK;
        break;
    case 'G':
        out->shape.color = GFX_COLOR_GRAY;
        break;
    default:
        out->shape.color = GFX_COLOR_WHITE;
    }
    out->shape.dimensions.width = width;
    out->shape.dimensions.height = height;
    cur++;

    return cur - str;
}

int
SldLoadScriptCall(const char *str, sld_entry *out)
{
    const char *cur = str;
    uint16_t    method, parameter;
    int         length;

    while (isspace(*cur))
    {
        cur++;
    }

    length = 0;
    while (!isspace(*cur))
    {
        if ((sizeof(out->script_call.file_name) - 1) < length)
        {
            ERR(SLD_CONTENT_TOO_LONG);
        }
        out->script_call.file_name[length] = *(cur++);
        length++;
    }
    out->script_call.file_name[length] = 0;
    out->length = length;

    if (('\r' == *cur) || ('\n' == *cur))
    {
        out->script_call.method = SLD_METHOD_STORE;
        out->script_call.crc32 = 0;
        out->script_call.parameter = 0;
    }
    else
    {
        cur += SldLoadU(cur, &out->script_call.method);
        out->script_call.crc32 = strtoul(cur, (char **)&cur, 16);
        cur += SldLoadU(cur, &out->script_call.parameter);
    }

    length = 0;
    while (('\r' != *cur) && ('\n' != *cur))
    {
        if ((sizeof(out->script_call.data) - 1) < length)
        {
            ERR(SLD_CONTENT_TOO_LONG);
        }
        out->script_call.data[length] = *(cur++);
        length++;
    }
    out->script_call.data[length] = 0;

    return cur - str;
}

int
SldLoadU(const char *str, uint16_t *out)
{
    const char *cur = str;
    *out = 0;

    while (isspace(*cur))
    {
        cur++;
    }

    while (isdigit(*cur))
    {
        *out *= 10;
        *out += *cur - '0';
        cur++;
    }

    if (!isspace(*cur))
    {
        return -1;
    }

    while (isspace(*cur))
    {
        cur++;
    }

    return cur - str;
}
