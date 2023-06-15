#include <stdlib.h>
#include <string.h>

#include <api/bios.h>
#include <dlg.h>
#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>

enum
{
    STATE_NONE,
    STATE_ALERT,
    STATE_PROMPT
};

enum
{
    STATE_PROMPT_NONE = STATE_PROMPT | (0 << 8),
    STATE_PROMPT_INVALID1 = STATE_PROMPT | (1 << 8),
    STATE_PROMPT_INVALID2 = STATE_PROMPT | (2 << 8),
    STATE_PROMPT_INVALID3 = STATE_PROMPT | (3 << 8)
};

static gfx_dimensions _screen = {0, 0};
static int            _state = STATE_NONE;
static uint32_t       _blink_start;
static char          *_buffer;
static int            _size;
static dlg_validator  _validator;
static int            _cursor;
static int            _field_left;
static int            _field_top;
static gfx_dimensions _box;
static int            _box_top;
static int            _box_left;

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
    gfx_draw_text("(C) 2021-2023", 1, 24);
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
    while (src[i])
    {
        if (delimiter == src[i])
        {
            dst[i] = 0;
            return i + 1;
        }

        if (count == i)
        {
            break;
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

static int
_handle_alert(void)
{
    uint16_t scancode = pal_get_keystroke();
    if (0 == scancode)
    {
        return DLG_INCOMPLETE;
    }

    if (0x01 == scancode)
    {
        _state = STATE_NONE;
        return DLG_CANCEL;
    }

    if (0x1C == scancode)
    {
        _state = STATE_NONE;
        return DLG_OK;
    }

    return DLG_INCOMPLETE;
}

int
dlg_alert(const char *title, const char *message)
{
    int title_length, columns, lines;
    _get_content_size(title, message, &title_length, &columns, &lines);
    _draw_background();
    _draw_frame(columns, lines, title, title_length);
    _draw_text(columns, lines, message);

    _state = STATE_ALERT;

    int status = DLG_INCOMPLETE;
    while (DLG_INCOMPLETE == status)
    {
        status = _handle_alert();
    }

    _state = STATE_NONE;
    return status;
}

static void
_draw_text_box(void)
{
    if (_validator && !_validator(_buffer))
    {
        gfx_draw_rectangle(&_box, _box_left, _box_top, GFX_COLOR_GRAY);
    }
    else
    {
        gfx_draw_rectangle(&_box, _box_left, _box_top, GFX_COLOR_BLACK);
    }

    gfx_fill_rectangle(&_box, _box_left, _box_top, GFX_COLOR_WHITE);
    gfx_draw_text(_buffer, _field_left, _field_top);
}

static int
_handle_prompt(void)
{
    if (STATE_PROMPT_INVALID1 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(63))
        {
            gfx_draw_rectangle(&_box, _box_left, _box_top, GFX_COLOR_GRAY);
            _state = STATE_PROMPT_INVALID2;
        }

        return DLG_INCOMPLETE;
    }

    if (STATE_PROMPT_INVALID2 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(126))
        {
            gfx_draw_rectangle(&_box, _box_left, _box_top, GFX_COLOR_BLACK);
            _state = STATE_PROMPT_INVALID3;
        }

        return DLG_INCOMPLETE;
    }

    if (STATE_PROMPT_INVALID3 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(189))
        {
            gfx_draw_rectangle(&_box, _box_left, _box_top, GFX_COLOR_GRAY);
            _draw_text_box();
            _state = STATE_PROMPT;
        }

        return DLG_INCOMPLETE;
    }

    uint16_t key = bios_check_keystroke();
    if (0 == key)
    {
        return DLG_INCOMPLETE;
    }

    bios_get_keystroke();
    uint8_t scancode = key >> 8, character = key & 0xFF;
    if (0x01 == scancode)
    {
        _state = STATE_NONE;
        return 0;
    }

    if (0x1C == scancode)
    {
        if (_validator && _validator(_buffer))
        {
            _state = STATE_NONE;
            return _cursor;
        }

        if (!_validator)
        {
            _state = STATE_NONE;
            return _cursor;
        }

        gfx_draw_rectangle(&_box, _box_left, _box_top, GFX_COLOR_BLACK);
        _blink_start = pal_get_counter();
        _state = STATE_PROMPT_INVALID1;
        return DLG_INCOMPLETE;
    }

    if ((0x0E == scancode) && (0 < _cursor))
    {
        _cursor--;
        _buffer[_cursor] = 0;
        _draw_text_box();
    }

    if ((0x20 <= character) && (0x80 > character) && (_cursor < _size))
    {
        _buffer[_cursor] = key & 0xFF;
        _cursor++;
        _buffer[_cursor] = 0;
        _draw_text_box();
    }

    return DLG_INCOMPLETE;
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
    _field_left = (_screen.width / 8 - field_width) / 2;
    _field_top = (_screen.height / 8 - 3 - lines) / 2 + 1 + lines;

    _box.width = field_width * 8 + 2;
    _box.height = 10;
    _box_top = _field_top * 8 - 1;
    _box_left = _field_left * 8 - 1;

    _draw_background();
    _draw_frame(columns, lines, title, title_length);
    _draw_text(columns, lines, message);

    _state = STATE_PROMPT;
    _buffer = buffer;
    _size = size;
    _validator = validator;
    _cursor = 0;

    _buffer[0] = 0;
    _draw_text_box();

    int status = DLG_INCOMPLETE;
    while (DLG_INCOMPLETE == status)
    {
        status = _handle_prompt();
    }

    _state = STATE_NONE;
    return status;
}
