#include <ctype.h>
#include <stdlib.h>

#include <api/dos.h>
#include <fmt/utf8.h>
#include <gfx.h>
#include <sld.h>

static int
_loadu(const char *str, uint16_t *out)
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

static int
_load_content(const char *str, sld_entry *out)
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

static int
_load_position(const char *str, sld_entry *out)
{
    const char *cur = str;
    int         length;

    // Load vertical position
    length = _loadu(cur, &out->posy);
    if (0 > length)
    {
        ERR(SLD_INVALID_VERTICAL);
    }
    cur += length;

    // Load horizontal position
    length = _loadu(cur, &out->posx);
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

static int
_load_shape(const char *str, sld_entry *out)
{
    const char *cur = str;
    uint16_t    width, height;

    cur += _loadu(cur, &width);
    cur += _loadu(cur, &height);

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

static int
_load_conditional(const char *str, sld_entry *out)
{
    const char *cur = str;
    int         length = 0;

    length = _loadu(cur, &out->posy);
    if (0 > length)
    {
        ERR(SLD_INVALID_COMPARISON);
    }
    cur += length;

    return cur - str;
}

static int
_load_script_call(const char *str, sld_entry *out)
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
        cur += _loadu(cur, &out->script_call.method);
        out->script_call.crc32 = strtoul(cur, (char **)&cur, 16);
        cur += _loadu(cur, &out->script_call.parameter);
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

static int
_convert_text(const char *str, sld_entry *inout)
{
    inout->length = utf8_encode(inout->content, inout->content, gfx_wctob);
    if (0 > inout->length)
    {
        ERR(KER_INVALID_SEQUENCE);
    }

    return 0;
}

#define _try_load(stage, str, out)                                             \
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
        _try_load(_load_content, cur, out);
        goto end;
    }

    // Load delay
    length = _loadu(cur, &num);
    if (0 > length)
    {
        ERR(SLD_INVALID_DELAY);
    }
    out->delay = num;
    cur += length;

    // Load type
    uint8_t type_tag = *cur;
    cur++;

    // Process all parts
    switch (type_tag)
    {
    case SLD_TAG_TYPE_TEXT:
        out->type = SLD_TYPE_TEXT;
        _try_load(_load_position, cur, out);
        _try_load(_load_content, cur, out);
        _try_load(_convert_text, cur, out);
        break;
    case SLD_TAG_TYPE_BITMAP:
        out->type = SLD_TYPE_BITMAP;
        _try_load(_load_position, cur, out);
        _try_load(_load_content, cur, out);
        break;
    case SLD_TAG_TYPE_RECT:
        out->type = SLD_TYPE_RECT;
        _try_load(_load_position, cur, out);
        _try_load(_load_shape, cur, out);
        break;
    case SLD_TAG_TYPE_RECTF:
        out->type = SLD_TYPE_RECTF;
        _try_load(_load_position, cur, out);
        _try_load(_load_shape, cur, out);
        break;
    case SLD_TAG_TYPE_PLAY:
        _try_load(_load_content, cur, out);
        out->type = SLD_TYPE_PLAY;
        break;
    case SLD_TAG_TYPE_WAITKEY:
        out->type = SLD_TYPE_WAITKEY;
        break;
    case SLD_TAG_TYPE_JUMP:
        out->type = SLD_TYPE_JUMP;
        _try_load(_load_content, cur, out);
        break;
    case SLD_TAG_TYPE_JUMPE:
        out->type = SLD_TYPE_JUMPE;
        _try_load(_load_conditional, cur, out);
        _try_load(_load_content, cur, out);
        break;
    case SLD_TAG_TYPE_CALL:
        out->type = SLD_TYPE_CALL;
        _try_load(_load_script_call, cur, out);
        break;
    default:
        ERR(SLD_UNKNOWN_TYPE);
    }

end:
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
