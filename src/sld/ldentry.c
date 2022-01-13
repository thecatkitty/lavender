#include <ctype.h>

#include <api/dos.h>
#include <ker.h>
#include <sld.h>
#include <vid.h>

static int
SldLoadU(const char *str, uint16_t *out);

extern int
SldLoadEntry(const char *line, SLD_ENTRY *out)
{
    const char *cur = line;
    if ('\r' == *cur)
    {
        out->Length = 0;
        return 0;
    }

    int      length;
    uint16_t num;

    // Load delay
    length = SldLoadU(cur, &num);
    if (0 > length)
    {
        ERR(SLD_INVALID_DELAY);
    }
    out->Delay = KerGetTicksFromMs(num);
    cur += length;

    // Load type
    switch (*cur)
    {
    case SLD_TAG_TYPE_TEXT:
        out->Type = SLD_TYPE_TEXT;
        break;
    case SLD_TAG_TYPE_BITMAP:
        out->Type = SLD_TYPE_BITMAP;
        break;
    default:
        ERR(SLD_UNKNOWN_TYPE);
    }
    cur++;

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

    // Load content
    char *content = out->Content;
    length = 0;
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

    if (*cur == '\r')
    {
        cur++;
    }
    cur++;
    out->Length = length;

    // Convert from UTF-8
    if (SLD_TYPE_TEXT == out->Type)
    {
        out->Length =
            KerConvertFromUtf8(out->Content, out->Content, VidConvertToLocal);
        if (0 > out->Length)
        {
            return out->Length;
        }
    }

    return cur - line;
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
