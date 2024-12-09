#include "direct.h"

static int
_measure_span(const char *str, size_t span)
{
#ifdef UTF8_NATIVE
    const char *end = str + span;
    int         length;

    for (length = 0; str < end; length++)
    {
        int seq;
        utf8_get_codepoint(str, &seq);
        str += seq;
    }

    return length;
#else
    return span;
#endif
}

static int
_copy_text(char *dst, const char *src, size_t length)
{
#ifdef UTF8_NATIVE
    int size = 0;

    for (int i = 0; i < length; i++)
    {
        int seq;
        utf8_get_codepoint(src + size, &seq);
        memcpy(dst + size, src + size, seq);
        size += seq;
    }

    dst[size] = 0;
    return size;
#else
    memcpy(dst, src, length);
    dst[length] = 0;
    return length;
#endif
}

static int
_wrap(char *dst, const char *src, size_t width, char delimiter)
{
    int chars = 0;

    const char *psrc = src;
    char       *pdst = dst;
    while (*psrc && (chars <= width))
    {
        if (delimiter == *psrc)
        {
            *pdst = 0;
            return psrc - src + 1;
        }

        size_t word_span = strcspn(psrc, " \n");
        int    word_length = _measure_span(psrc, word_span);
        if (width < chars + word_length)
        {
            if (src == psrc)
            {
                return _copy_text(dst, src, width);
            }

            break;
        }

        memcpy(pdst, psrc, word_span);
        psrc += word_span;
        pdst += word_span;
        chars += word_length;

        while ((' ' == *psrc) && (chars < width))
        {
            *pdst = *psrc;
            psrc++;
            pdst++;
            chars++;
        }

        if (chars == width)
        {
            while (' ' == *psrc)
            {
                psrc++;
            }
        }
    }

    *pdst = 0;
    return psrc - src;
}

int
encui_direct_print(int top, char *text)
{
#ifndef UTF8_NATIVE
    utf8_encode(text, text, pal_wctob);
#endif

    const char *fragment = text;
#ifdef UTF8_NATIVE
    char line_buff[2 * TEXT_WIDTH + 1];
#else
    char line_buff[TEXT_WIDTH + 1];
#endif

    int line = 0;
    while (*fragment)
    {
        fragment += _wrap(line_buff, fragment, TEXT_WIDTH, '\n');
        if (!gfx_draw_text(line_buff, 1, top + line))
        {
            return -1;
        }
        line++;
    }

    return line;
}
