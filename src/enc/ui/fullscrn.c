#include <stdlib.h>
#include <string.h>

#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>

#include "../../resource.h"
#include "../ui/encui.h"

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

#define TEXT_WIDTH (GFX_COLUMNS - 2)

// User interface state
static int             _state = STATE_NONE;
static char           *_buffer;
static int             _position;
static int             _length;
static int             _capacity;
static encui_validator _validator;

// Screen metrics
static gfx_dimensions _glyph = {0, 0};
static gfx_rect       _screen = {0, 0, 0, 0};

// Text box frame
static gfx_rect _tbox;
static int      _tbox_left;
static int      _tbox_top;
static uint32_t _tbox_blink_start;

// Caret animation
static uint32_t _caret_period = 0;
static uint32_t _caret_counter;
static bool     _caret_visible = true;

// Buttons
static gfx_rect _cancel;
static gfx_rect _ok;

static void
_draw_background(void)
{
    if (!_screen.width)
    {
        gfx_dimensions dim;
        gfx_get_screen_dimensions(&dim);
        gfx_get_glyph_dimensions(&_glyph);

        _screen.width = dim.width;
        _screen.height = dim.height;
    }

    gfx_fill_rectangle(&_screen, GFX_COLOR_WHITE);

    gfx_rect bar = {0, 0, _screen.width, 3 * _glyph.height + 1};
    bar.top = _screen.height - bar.height;
    gfx_fill_rectangle(&bar, GFX_COLOR_BLACK);

    gfx_draw_text(pal_get_version_string(), 1, 22);
    gfx_draw_text("https://celones.pl/lavender", 1, 23);
    gfx_draw_text("(C) 2021-2024 Mateusz Karcz", 1, 24);
}

static void
_draw_title(const char *title)
{
    gfx_rect bar = {0, 0, _screen.width, _glyph.height + 1};
    gfx_fill_rectangle(&bar, GFX_COLOR_BLACK);

#ifdef UTF8_NATIVE
    const char *title_l = title;
#else
    char *title_l = alloca(strlen(title) + 1);
    utf8_encode(title, title_l, pal_wctob);
#endif
    gfx_draw_text(title_l, 1, 0);
}

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
    int i = 0;
    int chars = 0;

    while (src[i])
    {
        if (delimiter == src[i])
        {
            dst[i] = 0;
            return i + 1;
        }

        size_t word_span = strcspn(src + i, " \n");
        int    word_length = _measure_span(src + i, word_span);
        if (width < chars + word_length)
        {
            if (0 == i)
            {
                return _copy_text(dst, src, width);
            }

            break;
        }

        memcpy(dst + i, src + i, word_span);
        i += word_span;
        chars += word_length;

        while (' ' == src[i])
        {
            dst[i] = src[i];
            i++;
            chars++;
        }
    }

    dst[i] = 0;
    return i;
}

static int
_draw_text(const char *text)
{
#ifdef UTF8_NATIVE
    const char *text_l = text;
#else
    char *text_l = alloca(strlen(text) + 1);
    utf8_encode(text, text_l, pal_wctob);
#endif

    const char *fragment = text_l;
#ifdef UTF8_NATIVE
    char line_buff[2 * TEXT_WIDTH + 1];
#else
    char line_buff[TEXT_WIDTH + 1];
#endif

    int line = 0;
    while (*fragment)
    {
        fragment += _wrap(line_buff, fragment, TEXT_WIDTH, '\n');
        if (!gfx_draw_text(line_buff, 1, 2 + line))
        {
            return -1;
        }
        line++;
    }

    return line;
}

static void
_draw_button(int x, int y, const char *text, gfx_rect *rect)
{
    rect->width = _glyph.width * 9;
    rect->height = _glyph.height * 3 / 2;
    rect->left = x * _glyph.width - (_glyph.width / 2);
    rect->top = y * _glyph.height - (_glyph.height / 4);

    gfx_rect inner = {rect->left - 1, rect->top, rect->width + 2, rect->height};
    gfx_fill_rectangle(rect, GFX_COLOR_WHITE);
    gfx_draw_rectangle(rect, GFX_COLOR_BLACK);
    gfx_draw_rectangle(&inner, GFX_COLOR_BLACK);
    gfx_draw_text(text, x + (8 - strlen(text)) / 2, y);
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
    else if (_is_pressed(&_cancel))
    {
        scancode = VK_ESCAPE;
    }

    if (0 == scancode)
    {
        scancode = pal_get_keystroke();
    }

    if (0 == scancode)
    {
        return ENCUI_INCOMPLETE;
    }

    if (VK_ESCAPE == scancode)
    {
        _reset();
        return ENCUI_CANCEL;
    }

    if (VK_RETURN == scancode)
    {
        _reset();
        return ENCUI_OK;
    }

    return ENCUI_INCOMPLETE;
}

bool
encui_alert(const char *title, const char *message)
{
    if (STATE_NONE != _state)
    {
        return false;
    }

    _draw_background();
    _draw_title(title);
    _draw_text(message);

    char caption[9];
    pal_load_string(IDS_OK, caption, sizeof(caption));
    _draw_button(GFX_COLUMNS - 10, GFX_LINES - 2, caption, &_ok);

    pal_enable_mouse();
    _state = STATE_ALERT;
    return true;
}

static void
_draw_text_box(void)
{
    if (_validator && !_validator(_buffer))
    {
        gfx_draw_rectangle(&_tbox, GFX_COLOR_GRAY);
    }
    else
    {
        gfx_draw_rectangle(&_tbox, GFX_COLOR_BLACK);
    }

    gfx_fill_rectangle(&_tbox, GFX_COLOR_WHITE);
    gfx_draw_text(_buffer, _tbox_left, _tbox_top);
}

static int
_handle_prompt(void)
{
    if (STATE_PROMPT_INVALID1 == _state)
    {
        if (pal_get_counter() > _tbox_blink_start + pal_get_ticks(63))
        {
            gfx_draw_rectangle(&_tbox, GFX_COLOR_GRAY);
            _state = STATE_PROMPT_INVALID2;
        }

        return ENCUI_INCOMPLETE;
    }

    if (STATE_PROMPT_INVALID2 == _state)
    {
        if (pal_get_counter() > _tbox_blink_start + pal_get_ticks(126))
        {
            gfx_draw_rectangle(&_tbox, GFX_COLOR_BLACK);
            _state = STATE_PROMPT_INVALID3;
        }

        return ENCUI_INCOMPLETE;
    }

    if (STATE_PROMPT_INVALID3 == _state)
    {
        if (pal_get_counter() > _tbox_blink_start + pal_get_ticks(189))
        {
            gfx_draw_rectangle(&_tbox, GFX_COLOR_GRAY);
            _draw_text_box();
            _state = STATE_PROMPT;
            pal_enable_mouse();
        }

        return ENCUI_INCOMPLETE;
    }

    gfx_rect caret = {(_tbox_left + _position) * _glyph.width, _tbox.top + 1, 1,
                      _glyph.height};
    if (pal_get_counter() > _caret_counter + _caret_period)
    {
        gfx_draw_line(&caret,
                      _caret_visible ? GFX_COLOR_BLACK : GFX_COLOR_WHITE);
        _caret_counter = pal_get_counter();
        _caret_visible = !_caret_visible;
    }

    uint16_t scancode = 0;
    if (_is_pressed(&_ok))
    {
        scancode = VK_RETURN;
    }
    else if (_is_pressed(&_cancel))
    {
        scancode = VK_ESCAPE;
    }
    else if (_is_pressed(&_tbox))
    {
        uint16_t x, y;
        pal_get_mouse(&x, &y);

        int cursor = x - _tbox_left;
        if (_length < cursor)
        {
            cursor = _length;
        }

        if (_position != cursor)
        {
            _position = cursor;
            caret.left = (_tbox_left + _position) * _glyph.width;
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
        return ENCUI_INCOMPLETE;
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
            return _length;
        }

        if (!_validator)
        {
            _reset();
            return _length;
        }

        pal_disable_mouse();
        gfx_draw_rectangle(&_tbox, GFX_COLOR_BLACK);
        _tbox_blink_start = pal_get_counter();
        _state = STATE_PROMPT_INVALID1;
        return ENCUI_INCOMPLETE;
    }

    pal_disable_mouse();
    gfx_draw_line(&caret, GFX_COLOR_WHITE);

    if ((VK_LEFT == scancode) && (0 < _position))
    {
        _position--;
        _draw_text_box();
    }

    if ((VK_RIGHT == scancode) && (_length > _position))
    {
        _position++;
        _draw_text_box();
    }

    if ((VK_BACK == scancode) && (0 < _position))
    {
        memmove(_buffer + _position - 1, _buffer + _position,
                _length - _position);
        _position--;
        _length--;
        _buffer[_length] = 0;
        _draw_text_box();
    }

    if ((VK_DELETE == scancode) && (_length > _position))
    {
        memmove(_buffer + _position, _buffer + _position + 1,
                _length - _position - 1);
        _length--;
        _buffer[_length] = 0;
        _draw_text_box();
    }

    if (((' ' == scancode) || (VK_OEM_MINUS == scancode) ||
         ((VK_DELETE < scancode) && (VK_F1 > scancode))) &&
        (_length < _capacity))
    {
        memmove(_buffer + _position + 1, _buffer + _position,
                _length - _position);
        _buffer[_position] =
            (VK_OEM_MINUS == scancode) ? '-' : (scancode & 0xFF);
        _position++;
        _length++;
        _buffer[_length] = 0;
        _draw_text_box();
    }

    caret.left = (_tbox_left + _position) * _glyph.width;
    gfx_draw_line(&caret, GFX_COLOR_BLACK);
    pal_enable_mouse();
    return ENCUI_INCOMPLETE;
}

bool
encui_prompt(const char     *title,
             const char     *message,
             char           *buffer,
             int             size,
             encui_validator validator)
{
    if (STATE_NONE != _state)
    {
        return false;
    }

    _draw_background();
    _draw_title(title);
    int lines = _draw_text(message);

    int field_width = 39;
    if (field_width < size)
    {
        field_width = size;
    }
    _tbox_left = 1;
    _tbox_top = 2 + lines + 2;

    _tbox.width = field_width * _glyph.width + 2;
    _tbox.height = _glyph.height + 2;
    _tbox.top = _tbox_top * _glyph.height - 1;
    _tbox.left = _tbox_left * _glyph.width - 1;

    _state = STATE_PROMPT;
    _buffer = buffer;
    _position = _length = 0;
    _capacity = size;
    _validator = validator;
    _caret_counter = pal_get_counter();
    _caret_period = pal_get_ticks(500);

    _buffer[0] = 0;
    _draw_text_box();

    char caption[9];
    pal_load_string(IDS_OK, caption, sizeof(caption));
    _draw_button(GFX_COLUMNS - 20, GFX_LINES - 2, caption, &_ok);
    pal_load_string(IDS_CANCEL, caption, sizeof(caption));
    _draw_button(GFX_COLUMNS - 10, GFX_LINES - 2, caption, &_cancel);

    pal_enable_mouse();
    return true;
}

int
encui_handle(void)
{
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
