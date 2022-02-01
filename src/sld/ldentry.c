#include <ctype.h>
#include <stdlib.h>

#include <api/dos.h>
#include <ker.h>
#include <sld.h>
#include <vid.h>

static int
SldLoadPosition(const char *str, SLD_ENTRY *out);

static int
SldLoadContent(const char *str, SLD_ENTRY *out);

static int
SldConvertText(const char *, SLD_ENTRY *inOut);

static int
SldLoadConditional(const char *str, SLD_ENTRY *out);

static int
SldLoadShape(const char *str, SLD_ENTRY *out);

static int
SldLoadScriptCall(const char *str, SLD_ENTRY *out);

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

extern int
SldLoadEntry(const char *line, SLD_ENTRY *out)
{
    const char *cur = line;
    if ('\r' == *cur)
    {
        out->Type = SLD_TYPE_BLANK;
        return ('\n' == cur[1]) ? 2 : 1;
    }

    int      length;
    uint16_t num;

    if (SLD_TAG_PREFIX_LABEL == *cur)
    {
        out->Type = SLD_TYPE_LABEL;
        out->Delay = 0;
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
    out->Delay = KerGetTicksFromMs(num);
    cur += length;

    // Load type
    uint8_t typeTag = *cur;
    cur++;

    // Process all parts
    switch (typeTag)
    {
    case SLD_TAG_TYPE_TEXT:
        out->Type = SLD_TYPE_TEXT;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        PROCESS_LOAD_PIPELINE(SldConvertText, cur, out);
        break;
    case SLD_TAG_TYPE_BITMAP:
        out->Type = SLD_TYPE_BITMAP;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        break;
    case SLD_TAG_TYPE_RECT:
        out->Type = SLD_TYPE_RECT;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadShape, cur, out);
        break;
    case SLD_TAG_TYPE_RECTF:
        out->Type = SLD_TYPE_RECTF;
        PROCESS_LOAD_PIPELINE(SldLoadPosition, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadShape, cur, out);
        break;
    case SLD_TAG_TYPE_PLAY:
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        out->Type = SLD_TYPE_PLAY;
        break;
    case SLD_TAG_TYPE_WAITKEY:
        out->Type = SLD_TYPE_WAITKEY;
        break;
    case SLD_TAG_TYPE_JUMP:
        out->Type = SLD_TYPE_JUMP;
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        break;
    case SLD_TAG_TYPE_JUMPE:
        out->Type = SLD_TYPE_JUMPE;
        PROCESS_LOAD_PIPELINE(SldLoadConditional, cur, out);
        PROCESS_LOAD_PIPELINE(SldLoadContent, cur, out);
        break;
    case SLD_TAG_TYPE_CALL:
        out->Type = SLD_TYPE_CALL;
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

    if (*cur == '\r')
    {
        cur++;
    }
    cur++;

    return cur - line;
}

int
SldLoadPosition(const char *str, SLD_ENTRY *out)
{
    const char *cur = str;
    int         length;

    // Load vertical position
    length = SldLoadU(cur, &out->Vertical);
    if (0 > length)
    {
        ERR(SLD_INVALID_VERTICAL);
    }
    cur += length;

    // Load horizontal position
    length = SldLoadU(cur, &out->Horizontal);
    if (0 <= length)
    {
        cur += length;
    }
    else
    {
        switch (*cur)
        {
        case SLD_TAG_ALIGN_LEFT:
            out->Horizontal = SLD_ALIGN_LEFT;
            break;
        case SLD_TAG_ALIGN_RIGHT:
            out->Horizontal = SLD_ALIGN_RIGHT;
            break;
        case SLD_TAG_ALIGN_CENTER:
            out->Horizontal = SLD_ALIGN_CENTER;
            break;
        default:
            ERR(SLD_INVALID_HORIZONTAL);
        }
        cur++;
    }

    return cur - str;
}

int
SldLoadContent(const char *str, SLD_ENTRY *out)
{
    const char *cur = str;
    char *      content = out->Content;
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

    out->Length = length;
    return cur - str;
}

int
SldConvertText(const char *str, SLD_ENTRY *inOut)
{
    inOut->Length =
        KerConvertFromUtf8(inOut->Content, inOut->Content, VidConvertToLocal);
    if (0 > inOut->Length)
    {
        return inOut->Length;
    }

    return 0;
}

int
SldLoadConditional(const char *str, SLD_ENTRY *out)
{
    const char *cur = str;
    int         length = 0;

    length = SldLoadU(cur, &out->Vertical);
    if (0 > length)
    {
        ERR(SLD_INVALID_COMPARISON);
    }
    cur += length;

    return cur - str;
}

int
SldLoadShape(const char *str, SLD_ENTRY *out)
{
    const char *cur = str;
    uint16_t    width, height;

    cur += SldLoadU(cur, &width);
    cur += SldLoadU(cur, &height);

    out->Shape.Color = ('W' == *cur) ? GFX_COLOR_WHITE : GFX_COLOR_BLACK;
    out->Shape.Dimensions.Width = width;
    out->Shape.Dimensions.Height = height;
    cur++;

    return cur - str;
}

int
SldLoadScriptCall(const char *str, SLD_ENTRY *out)
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
        if ((sizeof(out->ScriptCall.FileName) - 1) < length)
        {
            ERR(SLD_CONTENT_TOO_LONG);
        }
        out->ScriptCall.FileName[length] = *(cur++);
        length++;
    }
    out->ScriptCall.FileName[length] = 0;
    out->Length = length;

    if (('\r' == *cur) || ('\n' == *cur))
    {
        out->ScriptCall.Method = SLD_METHOD_STORE;
        out->ScriptCall.Crc32 = 0;
        out->ScriptCall.Parameter = 0;
    }
    else
    {
        cur += SldLoadU(cur, &out->ScriptCall.Method);
        out->ScriptCall.Crc32 = strtoul(cur, (char **)&cur, 16);
        cur += SldLoadU(cur, &out->ScriptCall.Parameter);
    }

    length = 0;
    while (('\r' != *cur) && ('\n' != *cur))
    {
        if ((sizeof(out->ScriptCall.Data) - 1) < length)
        {
            ERR(SLD_CONTENT_TOO_LONG);
        }
        out->ScriptCall.Data[length] = *(cur++);
        length++;
    }
    out->ScriptCall.Data[length] = 0;

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
