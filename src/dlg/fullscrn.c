#include <stdlib.h>
#include <string.h>

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

static gfx_dimensions _glyph = {0, 0};
static int            _state = STATE_NONE;
static uint32_t       _blink_start;
static char          *_buffer;
static dlg_validator  _validator;
static int            _field_left;
static int            _field_top;
static int            _cursor;
static int            _input_end;
static int            _size;
static uint32_t       _cursor_period = 0;
static uint32_t       _cursor_counter;
static bool           _cursor_visible = true;

static gfx_rect _screen = {0, 0, 0, 0};
static gfx_rect _box;
static gfx_rect _close;
static gfx_rect _ok;

static void
_draw_background(void)
{
    gfx_fill_rectangle(&_screen, GFX_COLOR_GRAY);

    gfx_rect bar = {0, 0, _screen.width, _glyph.height + 1};
    gfx_fill_rectangle(&bar, GFX_COLOR_WHITE);

    gfx_rect hline = {0, bar.height, _screen.width - 1, 1};
    gfx_draw_line(&hline, GFX_COLOR_BLACK);
    gfx_draw_text(pal_get_version_string(), 1, 0);

    bar.top = _screen.height - bar.height;
    gfx_fill_rectangle(&bar, GFX_COLOR_BLACK);
    gfx_draw_text("(C) 2021-2024", 1, 24);
    gfx_draw_text("https://celones.pl/lavender", 52, 24);
}

static void
_draw_frame(int columns, int lines, const char *title, int title_length)
{
    columns += 1 - (columns % 2);

    gfx_rect window = {0, 0, _glyph.width * (columns + 3),
                       _glyph.height * (lines + 6)};

    window.left = (_screen.width - window.width) / 2;
    window.top =
        (_screen.height - window.height) / 2 / _glyph.height * _glyph.height;
    gfx_fill_rectangle(&window, GFX_COLOR_WHITE);
    gfx_draw_rectangle(&window, GFX_COLOR_BLACK);
    window.left -= 1;
    window.width += 2;
    gfx_draw_rectangle(&window, GFX_COLOR_BLACK);
    window.left += 1;

    gfx_rect title_line = {window.left, window.top + _glyph.height + 1,
                           window.width - 1, 1};
    gfx_draw_line(&title_line, GFX_COLOR_BLACK);

    gfx_rect stripe = {window.left + window.width - 4, window.top + 1,
                       (window.width - ((title_length + 6) * _glyph.width)), 1};
    stripe.left -= stripe.width;
    for (int i = 0; i < _glyph.height; i += 2)
    {
        gfx_draw_line(&stripe, GFX_COLOR_BLACK);
        stripe.top += 2;
    }

#ifdef UTF8_NATIVE
    const char *title_l = title;
#else
    char *title_l = alloca(strlen(title) + 1);
    utf8_encode(title, title_l, pal_wctob);
#endif
    gfx_draw_text(title_l, window.left / _glyph.width + 4,
                  window.top / _glyph.height);
}

static int
_strndcpy(char *dst, const char *src, size_t count, char delimiter)
{
    int i = 0;
#ifdef UTF8_NATIVE
    int chars = 0;
#endif
    while (src[i])
    {
        if (delimiter == src[i])
        {
            dst[i] = 0;
            return i + 1;
        }

#ifdef UTF8_NATIVE
        if (count == chars)
#else
        if (count == i)
#endif
        {
            break;
        }

        dst[i] = src[i];
#ifdef UTF8_NATIVE
        if ((0x80 > (uint8_t)src[i]) || (0xBF < (uint8_t)src[i]))
        {
            ++chars;
        }
#endif

        ++i;
    }

    dst[i] = 0;
    return i;
}

static bool
_draw_text(int columns, int lines, const char *text)
{
    columns += 1 - (columns % 2);

#ifdef UTF8_NATIVE
    const char *text_l = text;
#else
    char *text_l = alloca(strlen(text) + 1);
    utf8_encode(text, text_l, pal_wctob);
#endif

    const char *fragment = text_l;
#ifdef UTF8_NATIVE
    char *line_buff = alloca(columns * 2 + 1);
#else
    char *line_buff = alloca(columns + 1);
#endif

    int left = (_screen.width / _glyph.width - columns) / 2;
    int top = (_screen.height / _glyph.height - 6 - lines) / 2 + 2;

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

static void
_draw_close(int columns, int lines)
{
    columns += 1 - (columns % 2);

    int left = (_screen.width / _glyph.width - columns) / 2 - 1;
    int top = (_screen.height / _glyph.height - 3 - lines) / 2 - 1;

    _close.width = _glyph.width * 3;
    _close.height = _glyph.height * 1 + 1;
    _close.left = left * _glyph.width;
    _close.top = top * _glyph.height;

    gfx_rect inner = {_close.left, _close.top, _close.width - 1, _close.height};
    gfx_fill_rectangle(&_close, GFX_COLOR_WHITE);
    gfx_draw_rectangle(&_close, GFX_COLOR_BLACK);
    gfx_draw_rectangle(&inner, GFX_COLOR_BLACK);
    gfx_draw_text("X", left + 1, top);
}

static void
_draw_ok(int columns, int lines)
{
    columns += 1 - (columns % 2);

    int left = (_screen.width / _glyph.width - columns) / 2;
    int top = (_screen.height / _glyph.height - 3 - lines) / 2 + 2;

    _ok.width = _glyph.width * 7;
    _ok.height = _glyph.height * 3 / 2;
    _ok.left = (left + columns - 7) * _glyph.width - (_glyph.width / 2);
    _ok.top = (top + lines + 1) * _glyph.height - (_glyph.height / 4);

    gfx_rect inner = {_ok.left - 1, _ok.top, _ok.width + 2, _ok.height};
    gfx_fill_rectangle(&_ok, GFX_COLOR_WHITE);
    gfx_draw_rectangle(&_ok, GFX_COLOR_BLACK);
    gfx_draw_rectangle(&inner, GFX_COLOR_BLACK);
    gfx_draw_text("Ok", left + columns - 5, top + lines + 1);
}

static bool
_is_pressed(const gfx_rect *rect)
{
    uint16_t msx, msy;
    if (0 == (PAL_MOUSE_LBUTTON & pal_get_mouse(&msx, &msy)))
    {
        return false;
    }

#if defined(__ia16__)
    msx *= 8;
    msy *= 8;
#else
    msx *= _glyph.width;
    msy *= _glyph.height;
#endif

    if ((rect->left > msx) || ((rect->left + rect->width) <= msx))
    {
        return false;
    }

    if ((rect->top > msy) || ((rect->top + rect->height) <= msy))
    {
        return false;
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
        gfx_dimensions dim;
        gfx_get_screen_dimensions(&dim);
        gfx_get_glyph_dimensions(&_glyph);

        _screen.width = dim.width;
        _screen.height = dim.height;
    }

    *head = _get_content_width(title, _screen.width / 10);
    *columns = _get_content_width(message, _screen.width / 10);
    *lines = _get_content_height(message);
}

static void
_reset(void)
{
    pal_disable_mouse();
    _state = STATE_NONE;
}

static int
_handle_alert(void)
{
    uint16_t scancode = 0;
    if (_is_pressed(&_ok))
    {
        scancode = VK_RETURN;
    }
    else if (_is_pressed(&_close))
    {
        scancode = VK_ESCAPE;
    }

    if (0 == scancode)
    {
        scancode = pal_get_keystroke();
    }

    if (0 == scancode)
    {
        return DLG_INCOMPLETE;
    }

    if (VK_ESCAPE == scancode)
    {
        _reset();
        return DLG_CANCEL;
    }

    if (VK_RETURN == scancode)
    {
        _reset();
        return DLG_OK;
    }

    return DLG_INCOMPLETE;
}

bool
dlg_alert(const char *title, const char *message)
{
    if (STATE_NONE != _state)
    {
        return false;
    }

    int title_length, columns, lines;
    _get_content_size(title, message, &title_length, &columns, &lines);
    _draw_background();
    _draw_frame(columns, lines, title, title_length);
    _draw_text(columns, lines, message);
    _draw_close(columns, lines);
    _draw_ok(columns, lines);

    pal_enable_mouse();
    _state = STATE_ALERT;
    return true;
}

static void
_draw_text_box(void)
{
    if (_validator && !_validator(_buffer))
    {
        gfx_draw_rectangle(&_box, GFX_COLOR_GRAY);
    }
    else
    {
        gfx_draw_rectangle(&_box, GFX_COLOR_BLACK);
    }

    gfx_fill_rectangle(&_box, GFX_COLOR_WHITE);
    gfx_draw_text(_buffer, _field_left, _field_top);
}

static int
_handle_prompt(void)
{
    if (STATE_PROMPT_INVALID1 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(63))
        {
            gfx_draw_rectangle(&_box, GFX_COLOR_GRAY);
            _state = STATE_PROMPT_INVALID2;
        }

        return DLG_INCOMPLETE;
    }

    if (STATE_PROMPT_INVALID2 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(126))
        {
            gfx_draw_rectangle(&_box, GFX_COLOR_BLACK);
            _state = STATE_PROMPT_INVALID3;
        }

        return DLG_INCOMPLETE;
    }

    if (STATE_PROMPT_INVALID3 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(189))
        {
            gfx_draw_rectangle(&_box, GFX_COLOR_GRAY);
            _draw_text_box();
            _state = STATE_PROMPT;
            pal_enable_mouse();
        }

        return DLG_INCOMPLETE;
    }

    gfx_rect cur = {(_field_left + _cursor) * _glyph.width, _box.top + 1, 1,
                    _glyph.height};
    if (pal_get_counter() > _cursor_counter + _cursor_period)
    {
        gfx_draw_line(&cur,
                      _cursor_visible ? GFX_COLOR_BLACK : GFX_COLOR_WHITE);
        _cursor_counter = pal_get_counter();
        _cursor_visible = !_cursor_visible;
    }

    uint16_t scancode = 0;
    if (_is_pressed(&_ok))
    {
        scancode = VK_RETURN;
    }
    else if (_is_pressed(&_close))
    {
        scancode = VK_ESCAPE;
    }
    else if (_is_pressed(&_box))
    {
        uint16_t x, y;
        pal_get_mouse(&x, &y);

        int cursor = x - _field_left;
        if (_input_end < cursor)
        {
            cursor = _input_end;
        }

        if (_cursor != cursor)
        {
            _cursor = cursor;
            cur.left = (_field_left + _cursor) * _glyph.width;
            pal_disable_mouse();
            _draw_text_box();
            pal_enable_mouse();
        }
    }

    if (0 == scancode)
    {
        scancode = pal_get_keystroke();
    }

    if (0 == scancode)
    {
        return DLG_INCOMPLETE;
    }

    if (VK_ESCAPE == scancode)
    {
        _reset();
        return 0;
    }

    if (VK_RETURN == scancode)
    {
        if (_validator && _validator(_buffer))
        {
            _reset();
            return _input_end;
        }

        if (!_validator)
        {
            _reset();
            return _input_end;
        }

        pal_disable_mouse();
        gfx_draw_rectangle(&_box, GFX_COLOR_BLACK);
        _blink_start = pal_get_counter();
        _state = STATE_PROMPT_INVALID1;
        return DLG_INCOMPLETE;
    }

    pal_disable_mouse();
    gfx_draw_line(&cur, GFX_COLOR_WHITE);

    if ((VK_LEFT == scancode) && (0 < _cursor))
    {
        _cursor--;
        _draw_text_box();
    }

    if ((VK_RIGHT == scancode) && (_input_end > _cursor))
    {
        _cursor++;
        _draw_text_box();
    }

    if ((VK_BACK == scancode) && (0 < _cursor))
    {
        memmove(_buffer + _cursor - 1, _buffer + _cursor, _input_end - _cursor);
        _cursor--;
        _input_end--;
        _buffer[_input_end] = 0;
        _draw_text_box();
    }

    if ((VK_DELETE == scancode) && (_input_end > _cursor))
    {
        memmove(_buffer + _cursor, _buffer + _cursor + 1,
                _input_end - _cursor - 1);
        _input_end--;
        _buffer[_input_end] = 0;
        _draw_text_box();
    }

    if (((' ' == scancode) || (VK_OEM_MINUS == scancode) ||
         ((VK_DELETE < scancode) && (VK_F1 > scancode))) &&
        (_input_end < _size))
    {
        memmove(_buffer + _cursor + 1, _buffer + _cursor, _input_end - _cursor);
        _buffer[_cursor] = (VK_OEM_MINUS == scancode) ? '-' : (scancode & 0xFF);
        _cursor++;
        _input_end++;
        _buffer[_input_end] = 0;
        _draw_text_box();
    }

    cur.left = (_field_left + _cursor) * _glyph.width;
    gfx_draw_line(&cur, GFX_COLOR_BLACK);
    pal_enable_mouse();
    return DLG_INCOMPLETE;
}

bool
dlg_prompt(const char   *title,
           const char   *message,
           char         *buffer,
           int           size,
           dlg_validator validator)
{
    if (STATE_NONE != _state)
    {
        return false;
    }

    int title_length, columns, lines;
    _get_content_size(title, message, &title_length, &columns, &lines);
    lines += 2;

    int field_width = (columns > size) ? size : columns;
    _field_left = (_screen.width / _glyph.width - field_width) / 2;
    _field_top = (_screen.height / _glyph.height - 3 - lines) / 2 + 1 + lines;

    _box.width = field_width * _glyph.width + 2;
    _box.height = _glyph.height + 2;
    _box.top = _field_top * _glyph.height - 1;
    _box.left = _field_left * _glyph.width - 1;

    _draw_background();
    _draw_frame(columns, lines, title, title_length);
    _draw_text(columns, lines, message);

    _state = STATE_PROMPT;
    _buffer = buffer;
    _size = size;
    _validator = validator;
    _cursor = _input_end = 0;
    _cursor_counter = pal_get_counter();

    _buffer[0] = 0;
    _draw_text_box();
    _draw_ok(columns, lines);
    _draw_close(columns, lines);

    pal_enable_mouse();
    return true;
}

int
dlg_handle(void)
{
    if (0 == _cursor_period)
    {
        _cursor_period = pal_get_ticks(500);
    }

    switch (_state & 0xFF)
    {
    case STATE_ALERT:
        return _handle_alert();

    case STATE_PROMPT:
        return _handle_prompt();

    default:
        return 0;
    }
}
