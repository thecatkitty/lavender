#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <dlg.h>
#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>

static gfx_dimensions _screen = {0, 0};

static void
_draw_background(void)
{
    gfx_fill_rectangle(&_screen, 0, 0, GFX_COLOR_GRAY);

    gfx_dimensions bar = {_screen.width, 9};
    gfx_fill_rectangle(&bar, 0, 0, GFX_COLOR_WHITE);

    gfx_dimensions hline = {_screen.width - 1, 1};
    gfx_draw_line(&hline, 0, bar.height, GFX_COLOR_BLACK);
    gfx_draw_text(pal_get_version_string(), 1, 0);

    gfx_fill_rectangle(&bar, 0, _screen.height - bar.height, GFX_COLOR_BLACK);
    gfx_draw_text("(C) 2021-2022", 1, 24);
    gfx_draw_text("https://github.com/thecatkitty/lavender/", 39, 24);
}

static void
_draw_frame(int columns, int lines, const char *title, int title_length)
{
    gfx_dimensions window = {8 * (columns + 3), 8 * (lines + 3)};

    int left = (_screen.width - window.width) / 2;
    int top = (_screen.height - window.height) / 16 * 8;
    gfx_fill_rectangle(&window, left, top, GFX_COLOR_WHITE);
    gfx_draw_rectangle(&window, left, top, GFX_COLOR_BLACK);
    window.width += 2;
    gfx_draw_rectangle(&window, left - 1, top, GFX_COLOR_BLACK);

    gfx_dimensions title_line = {window.width - 1, 1};
    gfx_draw_line(&title_line, left, top + 9, GFX_COLOR_BLACK);

    gfx_dimensions stripe = {(window.width - ((title_length + 2) * 8)) / 2 - 1,
                             1};
    for (int i = 0; i < 4; i++)
    {
        int y = top + 1 + i * 2;
        gfx_draw_line(&stripe, left + 1, y, GFX_COLOR_BLACK);
        gfx_draw_line(&stripe, left + window.width - stripe.width - 4, y,
                      GFX_COLOR_BLACK);
    }

    char *title_l = alloca(strlen(title) + 1);
    utf8_encode(title, title_l, gfx_wctob);
    gfx_draw_text(title_l, (_screen.width / 8 - title_length) / 2, top / 8);
}

static int
_strndcpy(char *dst, const char *src, size_t count, char delimiter)
{
    int i = 0;
    while (src[i] && (count > i))
    {
        if (delimiter == src[i])
        {
            dst[i] = 0;
            return i + 1;
        }

        dst[i] = src[i];
        ++i;
    }

    dst[i] = 0;
    return i;
}

static bool
_draw_text(int columns, int lines, const char *text)
{
    char *text_l = alloca(strlen(text) + 1);
    utf8_encode(text, text_l, gfx_wctob);

    const char *fragment = text_l;
    char       *line_buff = alloca(columns + 1);

    int left = (_screen.width / 8 - columns) / 2;
    int top = (_screen.height / 8 - 3 - lines) / 2 + 2;

    int line = 0;
    while (*fragment && (lines > line))
    {
        fragment += _strndcpy(line_buff, fragment, columns, '\n');
        if (!gfx_draw_text(line_buff, left, top + line))
        {
            return false;
        }
        line++;
    }

    return true;
}

static int
_get_content_width(const char *text, int limit)
{
    int max = 0, current = 0;
    while (*text)
    {
        int seq_length;
        if (L'\n' == utf8_get_codepoint(text, &seq_length))
        {
            if (max < current)
            {
                max = current;
            }

            current = 0;
        }

        text += (0 == seq_length) ? 1 : seq_length;
        current++;
    }

    if (max < current)
    {
        max = current;
    }

    if (limit && (limit < max))
    {
        return limit;
    }

    return max;
}

static int
_get_content_height(const char *text)
{
    int count = 1;
    while (*text)
    {
        if ('\n' == *text)
        {
            count++;
        }
        text++;
    }
    return count;
}

static void
_get_content_size(
    const char *title, const char *message, int *head, int *columns, int *lines)
{
    if (!_screen.width)
    {
        gfx_get_screen_dimensions(&_screen);
    }

    *head = _get_content_width(title, _screen.width / 10);
    *columns = _get_content_width(message, _screen.width / 10);
    *lines = _get_content_height(message);
}

int
dlg_alert(const char *title, const char *message)
{
    int title_length, columns, lines;
    _get_content_size(title, message, &title_length, &columns, &lines);
    _draw_background();
    _draw_frame(columns, lines, title, title_length);
    _draw_text(columns, lines, message);

    while (true)
    {
        uint16_t key = bios_get_keystroke();
        uint8_t  scancode = key >> 8;

        if (0x01 == scancode)
        {
            return DLG_CANCEL;
        }

        if (0x1C == scancode)
        {
            return DLG_OK;
        }
    }
}

int
dlg_prompt(const char   *title,
           const char   *message,
           char         *buffer,
           int           size,
           dlg_validator validator)
{
    int title_length, columns, lines;
    _get_content_size(title, message, &title_length, &columns, &lines);
    lines += 2;

    int field_width = (columns > size) ? size : columns;
    int field_left = (_screen.width / 8 - field_width) / 2;
    int field_top = (_screen.height / 8 - 3 - lines) / 2 + 1 + lines;

    gfx_dimensions box = {field_width * 8 + 2, 10};
    int            box_top = field_top * 8 - 1;
    int            box_left = field_left * 8 - 1;

    _draw_background();
    _draw_frame(columns, lines, title, title_length);
    _draw_text(columns, lines, message);

    gfx_draw_rectangle(&box, box_left, box_top, GFX_COLOR_BLACK);
    buffer[0] = 0;

    int cursor = 0;
    while (true)
    {
        if (validator)
        {
            gfx_draw_rectangle(&box, box_left, box_top,
                               validator(buffer) ? GFX_COLOR_BLACK
                                                 : GFX_COLOR_GRAY);
        }

        gfx_fill_rectangle(&box, box_left, box_top, GFX_COLOR_WHITE);
        gfx_draw_text(buffer, field_left, field_top);

        uint16_t key = bios_get_keystroke();
        uint8_t  scancode = key >> 8, character = key & 0xFF;
        if (0x01 == scancode)
        {
            return 0;
        }

        if (0x1C == scancode)
        {
            if (validator && validator(buffer))
            {
                return cursor;
            }

            if (!validator)
            {
                return cursor;
            }

            gfx_draw_rectangle(&box, box_left, box_top, GFX_COLOR_BLACK);
            pal_sleep(63);
            gfx_draw_rectangle(&box, box_left, box_top, GFX_COLOR_GRAY);
            pal_sleep(63);
            gfx_draw_rectangle(&box, box_left, box_top, GFX_COLOR_BLACK);
            pal_sleep(63);
            continue;
        }

        if ((0x0E == scancode) && (0 < cursor))
        {
            cursor--;
            buffer[cursor] = 0;
        }

        if ((0x20 <= character) && (0x80 > character) && (cursor < size))
        {
            buffer[cursor] = key & 0xFF;
            cursor++;
            buffer[cursor] = 0;
        }
    }
}
