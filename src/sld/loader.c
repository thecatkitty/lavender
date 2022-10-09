#include <ctype.h>
#include <string.h>

#include "sld_impl.h"

int
__sld_loadu(const char *str, uint16_t *out)
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

int
__sld_load_content(const char *str, sld_entry *out)
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
            strncpy((char *)out, IDS_LONGCONTENT, sizeof(sld_entry));
            return SLD_ARGERR;
        }
        *(content++) = *(cur++);
        length++;
    }
    *content = 0;

    out->length = length;
    return cur - str;
}

int
__sld_load_position(const char *str, sld_entry *out)
{
    const char *cur = str;
    int         length;

    // Load vertical position
    length = __sld_loadu(cur, &out->posy);
    if (0 > length)
    {
        strncpy((char *)out, IDS_INVALIDVPOS, sizeof(sld_entry));
        return SLD_ARGERR;
    }
    cur += length;

    // Load horizontal position
    length = __sld_loadu(cur, &out->posx);
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
            strncpy((char *)out, IDS_INVALIDHPOS, sizeof(sld_entry));
            return SLD_ARGERR;
        }
        cur++;
    }

    return cur - str;
}

int
sld_load_entry(sld_context *ctx, sld_entry *out)
{
    const char *start = ctx->data + ctx->offset;
    const char *cur = start;
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
        __sld_try_load(__sld_load_content, cur, out);
        goto end;
    }

    // Load delay
    length = __sld_loadu(cur, &num);
    if (0 > length)
    {
        strncpy((char *)out, IDS_INVALIDDELAY, sizeof(sld_entry));
        return SLD_ARGERR;
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
        length = __sld_load_text(cur, out);
        break;
    case SLD_TAG_TYPE_BITMAP:
        out->type = SLD_TYPE_BITMAP;
        length = __sld_load_bitmap(cur, out);
        break;
    case SLD_TAG_TYPE_RECT:
        out->type = SLD_TYPE_RECT;
        length = __sld_load_shape(cur, out);
        break;
    case SLD_TAG_TYPE_RECTF:
        out->type = SLD_TYPE_RECTF;
        length = __sld_load_shape(cur, out);
        break;
    case SLD_TAG_TYPE_PLAY:
        out->type = SLD_TYPE_PLAY;
        length = __sld_load_content(cur, out);
        break;
    case SLD_TAG_TYPE_WAITKEY:
        out->type = SLD_TYPE_WAITKEY;
        break;
    case SLD_TAG_TYPE_JUMP:
        out->type = SLD_TYPE_JUMP;
        length = __sld_load_content(cur, out);
        break;
    case SLD_TAG_TYPE_JUMPE:
        out->type = SLD_TYPE_JUMPE;
        length = __sld_load_conditional(cur, out);
        break;
    case SLD_TAG_TYPE_CALL:
        out->type = SLD_TYPE_CALL;
        length = __sld_load_script_call(cur, out);
        break;
    default:
        strncpy((char *)out, IDS_UNKNOWNTYPE, sizeof(sld_entry));
        return SLD_ARGERR;
    }

    if (0 > length)
    {
        return length;
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

    return cur - start;
}
