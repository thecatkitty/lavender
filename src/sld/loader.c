#include <ctype.h>

#include "sld_impl.h"

int
__sld_loadu(const char *str, uint16_t *out)
{
    const char *cur = str;
    *out = 0;

    while (isspace((uint8_t)*cur))
    {
        cur++;
    }

    while (isdigit((uint8_t)*cur))
    {
        *out *= 10;
        *out += *cur - '0';
        cur++;
    }

    if (!isspace((uint8_t)*cur))
    {
        return -1;
    }

    while (isspace((uint8_t)*cur))
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

    while (isspace((uint8_t)*cur))
    {
        cur++;
    }

    while (('\r' != *cur) && ('\n' != *cur))
    {
        if (SLD_ENTRY_MAX_LENGTH < length)
        {
            __sld_errmsgcpy(out, IDS_LONGCONTENT);
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
        __sld_errmsgcpy(out, IDS_INVALIDVPOS);
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
            __sld_errmsgcpy(out, IDS_INVALIDHPOS);
            return SLD_ARGERR;
        }
        cur++;
    }

    return cur - str;
}

int
sld_load_entry(sld_context *ctx, sld_entry *out)
{
    const char *start = (char *)ctx->data + ctx->offset;
    const char *cur = start;
    int         length;
    uint16_t    num;
    uint8_t     type_tag;

    if ('\r' == *cur)
    {
        out->type = SLD_TYPE_BLANK;
        return ('\n' == cur[1]) ? 2 : 1;
    }

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
        __sld_errmsgcpy(out, IDS_INVALIDDELAY);
        return SLD_ARGERR;
    }
    out->delay = num;
    cur += length;

    // Load type
    type_tag = *cur;
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
    case SLD_TAG_TYPE_ACTAREA:
        out->type = SLD_TYPE_ACTAREA;
        length = __sld_load_active_area(cur, out);
        break;
    case SLD_TAG_TYPE_QUERY:
        out->type = SLD_TYPE_QUERY;
        length = __sld_load_content(cur, out);
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
        __sld_errmsgcpy(out, IDS_UNKNOWNTYPE);
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
