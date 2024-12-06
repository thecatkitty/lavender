#include <string.h>

#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>

#include "../../resource.h"
#include "../ui/encui.h"

enum
{
    STATE_NONE,
    STATE_PROMPT,
    STATE_PROMPT_INVALID1,
    STATE_PROMPT_INVALID2,
    STATE_PROMPT_INVALID3,
    STATE_VERIFY,
};

#define TEXT_WIDTH (GFX_COLUMNS - 2)

// User interface state
static int         _state = STATE_NONE;
static int         _position;
static encui_page *_pages = NULL;
static int         _id;

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
static gfx_rect _back;
static gfx_rect _next;

// Check box
static gfx_rect _checkbox = {0};
static gfx_rect _checkbox_area;
static bool     _checkbox_down;

bool
encui_enter(encui_page *pages, int count)
{
    if (STATE_NONE != _state)
    {
        return false;
    }

    gfx_dimensions dim;
    gfx_get_screen_dimensions(&dim);
    gfx_get_glyph_dimensions(&_glyph);

    _screen.width = dim.width;
    _screen.height = dim.height;

    gfx_rect bar = {0, 0, _screen.width, 3 * _glyph.height + 1};
    bar.top = _screen.height - bar.height;
    gfx_fill_rectangle(&bar, GFX_COLOR_BLACK);

    gfx_draw_text(pal_get_version_string(), 1, 22);
    gfx_draw_text("https://celones.pl/lavender", 1, 23);
    gfx_draw_text("(C) 2021-2024 Mateusz Karcz", 1, 24);

    _pages = pages;
    _id = -1;
    return true;
}

bool
encui_exit(void)
{
    _state = STATE_NONE;
    return true;
}

static void
_draw_title(char *title)
{
    gfx_rect bar = {0, 0, _screen.width, _glyph.height + 1};
    gfx_fill_rectangle(&bar, GFX_COLOR_BLACK);

#ifndef UTF8_NATIVE
    utf8_encode(title, title, pal_wctob);
#endif
    gfx_draw_text(title, 1, 0);
}

static void
_draw_background(void)
{
    gfx_rect bg = {0, _glyph.height, _screen.width,
                   (GFX_LINES - 4) * _glyph.height};
    gfx_fill_rectangle(&bg, GFX_COLOR_WHITE);

    gfx_rect footer = {0, 0, _screen.width / 2, 3 * _glyph.height};
    footer.left = footer.width;
    footer.top = _screen.height - footer.height;
    gfx_fill_rectangle(&footer, GFX_COLOR_BLACK);
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

static int
_draw_text(int top, char *text)
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

static void
_draw_button(int x, int y, const char *text, gfx_rect *rect)
{
    rect->width = _glyph.width * 9;
    rect->height = _glyph.height * 3 / 2;
    rect->left = x * _glyph.width - (_glyph.width / 2);
    rect->top = y * _glyph.height - (_glyph.height / 4);

#ifdef UTF8_NATIVE
    const char *buff = text;
#else
    char buff[9];
    utf8_encode(text, buff, pal_wctob);
#endif

    gfx_rect inner = {rect->left - 1, rect->top, rect->width + 2, rect->height};
    gfx_fill_rectangle(rect, GFX_COLOR_WHITE);
    gfx_draw_rectangle(rect, GFX_COLOR_BLACK);
    gfx_draw_rectangle(&inner, GFX_COLOR_BLACK);
    gfx_draw_text(buff, x + (8 - strlen(buff)) / 2, y);
}

static bool
_is_pressed(const gfx_rect *rect)
{
    uint16_t msx, msy;
    if (0 == (PAL_MOUSE_LBUTTON & pal_get_mouse(&msx, &msy)))
    {
        return false;
    }

    if (0 == rect->width)
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

static void
_draw_check_box(void)
{
    encui_field *checkbox = encui_find_checkbox(_pages + _id);
    if (NULL == checkbox)
    {
        return;
    }

#ifndef __ia16__
    gfx_fill_rectangle(&_checkbox, GFX_COLOR_WHITE);

    if (ENCUIFF_CHECKED & checkbox->flags)
#endif
    {
        gfx_draw_text("x", 2, GFX_LINES - 5);
    }
}

static void
_draw_text_box(void)
{
    encui_prompt_page *prompt = encui_find_prompt(_pages + _id);
    if (0 < _pages[_id].proc(ENCUIM_CHECK, prompt->buffer, _pages[_id].data))
    {
        gfx_draw_rectangle(&_tbox, GFX_COLOR_GRAY);
    }
    else
    {
        gfx_draw_rectangle(&_tbox, GFX_COLOR_BLACK);
    }

    gfx_fill_rectangle(&_tbox, GFX_COLOR_WHITE);
    gfx_draw_text(prompt->buffer, _tbox_left, _tbox_top);
}

int
encui_handle(void)
{
    encui_page        *page = _pages + _id;
    encui_prompt_page *prompt = encui_find_prompt(_pages + _id);

    if (STATE_NONE == _state)
    {
        return 0;
    }

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

    if (STATE_VERIFY == _state)
    {
        int status = page->proc(ENCUIM_NEXT, prompt->buffer, page->data);
        if ((0 == status) || (-ENOSYS == status))
        {
            _reset();
            return prompt->length;
        }

        if (0 > status)
        {
            return status;
        }

        gfx_rect bg = {0, (_tbox_top + 2) * _glyph.height, _screen.width, 0};
        bg.height = (GFX_LINES - 3) * _glyph.height - bg.top;
        gfx_fill_rectangle(&bg, GFX_COLOR_WHITE);

        char message[GFX_COLUMNS];
        if (INT_MAX == status)
        {
            strncpy(message, prompt->alert, GFX_COLUMNS - 1);
            free((void *)prompt->alert);
            prompt->alert = NULL;
        }
        else
        {
            pal_load_string(status, message, sizeof(message));
        }
        _draw_text(_tbox_top + 2, message);

        _state = STATE_PROMPT;
        pal_enable_mouse();
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
    if (_is_pressed(&_back))
    {
        scancode = VK_PRIOR;
    }
    else if (_is_pressed(&_next))
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
        if (prompt->length < cursor)
        {
            cursor = prompt->length;
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
    else if (_is_pressed(&_checkbox_area))
    {
        scancode = VK_F8;
    }

    if (0 == scancode)
    {
        scancode = pal_get_keystroke();
    }

    if (0 == scancode)
    {
        _checkbox_down = false;
        return ENCUI_INCOMPLETE;
    }

    if (VK_ESCAPE == scancode)
    {
        _reset();
        return 0;
    }

    if (VK_RETURN == scancode)
    {
        if (0 >= page->proc(ENCUIM_CHECK, prompt->buffer, page->data))
        {
            _state = STATE_VERIFY;
            return ENCUI_INCOMPLETE;
        }

        pal_disable_mouse();
        gfx_draw_rectangle(&_tbox, GFX_COLOR_BLACK);
        _tbox_blink_start = pal_get_counter();
        _state = STATE_PROMPT_INVALID1;
        return ENCUI_INCOMPLETE;
    }

    if ((VK_PRIOR == scancode) && (0 < _id))
    {
        _reset();
        encui_set_page(_id - 1);
        return ENCUI_INCOMPLETE;
    }

    if (VK_F8 == scancode)
    {
        encui_field *checkbox = encui_find_checkbox(_pages + _id);
        if ((NULL != checkbox) && !_checkbox_down)
        {
            _checkbox_down = true;
            checkbox->flags ^= ENCUIFF_CHECKED;
            _draw_check_box();
        }
    }

    pal_disable_mouse();
    gfx_draw_line(&caret, GFX_COLOR_WHITE);

    if ((VK_LEFT == scancode) && (0 < _position))
    {
        _position--;
        _draw_text_box();
    }

    if ((VK_RIGHT == scancode) && (prompt->length > _position))
    {
        _position++;
        _draw_text_box();
    }

    if ((VK_BACK == scancode) && (0 < _position))
    {
        memmove(prompt->buffer + _position - 1, prompt->buffer + _position,
                prompt->length - _position);
        _position--;
        prompt->length--;
        prompt->buffer[prompt->length] = 0;
        _draw_text_box();
    }

    if ((VK_DELETE == scancode) && (prompt->length > _position))
    {
        memmove(prompt->buffer + _position, prompt->buffer + _position + 1,
                prompt->length - _position - 1);
        prompt->length--;
        prompt->buffer[prompt->length] = 0;
        _draw_text_box();
    }

    if (((' ' == scancode) || (VK_OEM_MINUS == scancode) ||
         ((VK_DELETE < scancode) && (VK_F1 > scancode))) &&
        (prompt->length < prompt->capacity))
    {
        memmove(prompt->buffer + _position + 1, prompt->buffer + _position,
                prompt->length - _position);
        prompt->buffer[_position] =
            (VK_OEM_MINUS == scancode) ? '-' : (scancode & 0xFF);
        _position++;
        prompt->length++;
        prompt->buffer[prompt->length] = 0;
        _draw_text_box();
    }

    caret.left = (_tbox_left + _position) * _glyph.width;
    gfx_draw_line(&caret, GFX_COLOR_BLACK);
    pal_enable_mouse();
    return ENCUI_INCOMPLETE;
}

int
encui_get_page(void)
{
    return _id;
}

static void
_create_text_box(encui_prompt_page *prompt, int *cy)
{
    if (0 == prompt->length)
    {
        prompt->buffer[0] = 0;
    }

    int field_width = GFX_COLUMNS / 2 - 1;
    if (field_width < prompt->capacity)
    {
        field_width = prompt->capacity;
    }

    (*cy)++;
    _tbox_left = 1;
    _tbox_top = *cy;

    _tbox.width = field_width * _glyph.width + 2;
    _tbox.height = _glyph.height + 2;
    _tbox.top = _tbox_top * _glyph.height - 1;
    _tbox.left = _tbox_left * _glyph.width - 1;

    _state = STATE_PROMPT;
    _position = prompt->length;
    _caret_counter = pal_get_counter();
    _caret_period = pal_get_ticks(500);

    _draw_text_box();

    (*cy) += 3;
}

static void
_create_controls(encui_page *page)
{
    char buffer[GFX_COLUMNS * 4];
    int  cy = 2;
    bool has_checkbox = false;

    for (int i = 0; i < page->length; i++)
    {
        encui_field *field = &page->fields[i];

        if (ENCUIFT_SEPARATOR == field->type)
        {
            cy += field->data;
        }

        if (ENCUIFT_LABEL == field->type)
        {
            if (ENCUIFF_DYNAMIC & field->flags)
            {
                strncpy(buffer, (const char *)field->data, sizeof(buffer));
            }
            else
            {
                pal_load_string(field->data, buffer, sizeof(buffer));
            }
            cy += _draw_text(cy, buffer) + 1;
        }

        if (ENCUIFT_TEXTBOX == field->type)
        {
            _create_text_box((encui_prompt_page *)field->data, &cy);
        }

        if (ENCUIFT_CHECKBOX == field->type)
        {
            if (has_checkbox)
            {
                continue;
            }

            has_checkbox = true;
            _checkbox_down = false;
            buffer[0] = buffer[1] = buffer[2] = ' ';
            if (ENCUIFF_DYNAMIC & field->flags)
            {
                strncpy(buffer + 3, (const char *)field->data,
                        sizeof(buffer) - 8);
            }
            else
            {
                pal_load_string(field->data, buffer + 3, sizeof(buffer) - 8);
            }
            strcat(buffer, " [F8]");
            _draw_text(GFX_LINES - 5, buffer);

            _checkbox.width = _checkbox.height = _glyph.width + 4;
            _checkbox.left = _glyph.width * 5 / 3 + 1;
            _checkbox.top = (GFX_LINES - 5) * _glyph.height +
                            (_glyph.height - _glyph.width) / 2;
            gfx_draw_rectangle(&_checkbox, GFX_COLOR_BLACK);
            if (ENCUIFF_CHECKED & field->flags)
            {
                _draw_check_box();
            }

            _checkbox_area.left = _glyph.width;
            _checkbox_area.top = _checkbox.top - (_glyph.height / 2);
            _checkbox_area.width = TEXT_WIDTH * _glyph.width;
            _checkbox_area.height = _glyph.height * 2;
        }
    }
}

bool
encui_set_page(int id)
{
    if (_id == id)
    {
        return true;
    }

    if (STATE_NONE != _state)
    {
        return false;
    }

    encui_page *page = _pages + id;
    _id = id;

    if (0 == page->title)
    {
        _id = -1;
        _state = STATE_NONE;
        return true;
    }

    char buffer[GFX_COLUMNS * 2];
    _draw_background();
    pal_load_string(page->title, buffer, sizeof(buffer));
    _draw_title(buffer);
    _create_controls(page);

    char caption[9];
    if ((0 < id) && (0 != _pages[id - 1].title))
    {
        pal_load_string(IDS_BACK, caption, sizeof(caption));
        _draw_button(GFX_COLUMNS - 31, GFX_LINES - 2, caption, &_back);
    }
    else
    {
        _back.width = 0;
    }
    pal_load_string(IDS_NEXT, caption, sizeof(caption));
    _draw_button(GFX_COLUMNS - 21, GFX_LINES - 2, caption, &_next);
    pal_load_string(IDS_CANCEL, caption, sizeof(caption));
    _draw_button(GFX_COLUMNS - 10, GFX_LINES - 2, caption, &_cancel);

    pal_enable_mouse();
    return true;
}
