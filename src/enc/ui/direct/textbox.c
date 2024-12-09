#include "direct.h"

enum
{
    STATE_PROMPT,
    STATE_INVALID1,
    STATE_INVALID2,
    STATE_INVALID3,
};

static int _state;
static int _position;

// Text box frame
static gfx_rect _area;
static int      _left;
static int      _top;
static uint32_t _blink_start;

// Caret animation
static uint32_t _caret_period = 0;
static uint32_t _caret_counter;
static bool     _caret_visible = true;

void
encui_direct_create_textbox(encui_prompt_page *prompt,
                            encui_page        *page,
                            int               *cy)
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
    _left = 1;
    _top = *cy;

    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);
    _area.width = field_width * glyph.width + 2;
    _area.height = glyph.height + 2;
    _area.top = _top * glyph.height - 1;
    _area.left = _left * glyph.width - 1;

    _state = STATE_PROMPT;
    _position = prompt->length;
    _caret_counter = pal_get_counter();
    _caret_period = pal_get_ticks(500);

    encui_direct_draw_textbox(prompt, page);

    (*cy) += 3;
}

void
encui_direct_draw_textbox(encui_prompt_page *prompt, encui_page *page)
{
    if (NULL == prompt)
    {
        return;
    }

    if (0 < page->proc(ENCUIM_CHECK, prompt->buffer, page->data))
    {
        gfx_draw_rectangle(&_area, GFX_COLOR_GRAY);
    }
    else
    {
        gfx_draw_rectangle(&_area, GFX_COLOR_BLACK);
    }

    gfx_fill_rectangle(&_area, GFX_COLOR_WHITE);
    gfx_draw_text(prompt->buffer, _left, _top);
}

static void
_calculate_caret(gfx_rect *caret)
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    caret->left = (_left + _position) * glyph.width;
    caret->top = _area.top + 1;
    caret->width = 1;
    caret->height = glyph.height;
}

bool
encui_direct_animate_textbox(encui_prompt_page *prompt,
                             encui_page        *page,
                             bool               valid)
{
    if (!valid && (STATE_PROMPT == _state))
    {
        _state = STATE_INVALID1;
        _blink_start = pal_get_counter();
        return false;
    }

    if (STATE_INVALID1 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(63))
        {
            gfx_draw_rectangle(&_area, GFX_COLOR_GRAY);
            _state = STATE_INVALID2;
        }

        return false;
    }

    if (STATE_INVALID2 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(126))
        {
            gfx_draw_rectangle(&_area, GFX_COLOR_BLACK);
            _state = STATE_INVALID3;
        }

        return false;
    }

    if (STATE_INVALID3 == _state)
    {
        if (pal_get_counter() > _blink_start + pal_get_ticks(189))
        {
            gfx_draw_rectangle(&_area, GFX_COLOR_GRAY);
            encui_direct_draw_textbox(prompt, page);
            _state = STATE_PROMPT;
            pal_enable_mouse();
        }

        return false;
    }

    gfx_rect caret;
    _calculate_caret(&caret);
    if (pal_get_counter() > _caret_counter + _caret_period)
    {
        gfx_draw_line(&caret,
                      _caret_visible ? GFX_COLOR_BLACK : GFX_COLOR_WHITE);
        _caret_counter = pal_get_counter();
        _caret_visible = !_caret_visible;
    }

    return true;
}

void
encui_direct_set_textbox_error(char *message)
{
    gfx_dimensions glyph, screen;
    gfx_get_glyph_dimensions(&glyph);
    gfx_get_screen_dimensions(&screen);

    gfx_rect bg = {0, (_top + 2) * glyph.height, screen.width, 0};
    bg.height = (GFX_LINES - 3) * glyph.height - bg.top;
    gfx_fill_rectangle(&bg, GFX_COLOR_WHITE);

    encui_direct_print(_top + 2, message);
}

const gfx_rect *
encui_direct_get_textbox_area(void)
{
    return &_area;
}

void
encui_direct_click_textbox(encui_prompt_page *prompt,
                           encui_page        *page,
                           uint16_t           x,
                           uint16_t           y)
{
    int cursor = x - _left;
    if (prompt->length < cursor)
    {
        cursor = prompt->length;
    }

    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    gfx_rect caret;
    _calculate_caret(&caret);
    if (_position != cursor)
    {
        _position = cursor;
        caret.left = (_left + _position) * glyph.width;
        pal_disable_mouse();
        encui_direct_draw_textbox(prompt, page);
        pal_enable_mouse();
    }
}

void
encui_direct_key_textbox(encui_prompt_page *prompt,
                         encui_page        *page,
                         uint16_t           scancode)
{
    pal_disable_mouse();

    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    gfx_rect caret;
    _calculate_caret(&caret);
    gfx_draw_line(&caret, GFX_COLOR_WHITE);

    if ((VK_LEFT == scancode) && (0 < _position))
    {
        _position--;
        encui_direct_draw_textbox(prompt, page);
    }

    if ((VK_RIGHT == scancode) && (prompt->length > _position))
    {
        _position++;
        encui_direct_draw_textbox(prompt, page);
    }

    if ((VK_BACK == scancode) && (0 < _position))
    {
        memmove(prompt->buffer + _position - 1, prompt->buffer + _position,
                prompt->length - _position);
        _position--;
        prompt->length--;
        prompt->buffer[prompt->length] = 0;
        encui_direct_draw_textbox(prompt, page);
    }

    if ((VK_DELETE == scancode) && (prompt->length > _position))
    {
        memmove(prompt->buffer + _position, prompt->buffer + _position + 1,
                prompt->length - _position - 1);
        prompt->length--;
        prompt->buffer[prompt->length] = 0;
        encui_direct_draw_textbox(prompt, page);
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
        encui_direct_draw_textbox(prompt, page);
    }

    caret.left = (_left + _position) * glyph.width;
    gfx_draw_line(&caret, GFX_COLOR_BLACK);
    pal_enable_mouse();
}
