#include <string.h>

#include <fmt/utf8.h>
#include <gfx.h>
#include <pal.h>

#include "../../../resource.h"
#include "direct.h"

enum
{
    STATE_NONE,
    STATE_PROMPT,
    STATE_VERIFY,
};

// User interface state
static int         _state = STATE_NONE;
static encui_page *_pages = NULL;
static int         _id;

// Screen metrics
static gfx_dimensions _glyph = {0, 0};
static gfx_rect       _screen = {0, 0, 0, 0};

// Buttons
static gfx_rect _cancel;
static gfx_rect _back;
static gfx_rect _next;
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

int
encui_handle(void)
{
    encui_page        *page = _pages + _id;
    encui_prompt_page *prompt = encui_find_prompt(_pages + _id);

    if (STATE_NONE == _state)
    {
        return 0;
    }

    if (!encui_direct_animate_textbox(prompt, page, true))
    {
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
        encui_direct_set_textbox_error(message);

        _state = STATE_PROMPT;
        pal_enable_mouse();
        return ENCUI_INCOMPLETE;
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
    else if (_is_pressed(encui_direct_get_textbox_area()))
    {
        uint16_t x, y;
        pal_get_mouse(&x, &y);
        encui_direct_click_textbox(prompt, page, x, y);
    }
    else if (_is_pressed(encui_direct_get_checkbox_area()))
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
        gfx_draw_rectangle(encui_direct_get_textbox_area(), GFX_COLOR_BLACK);
        encui_direct_animate_textbox(prompt, page, false);
        return ENCUI_INCOMPLETE;
    }

    if ((VK_PRIOR == scancode) && (0 < _id))
    {
        _reset();
        encui_set_page(_id - 1);
        return ENCUI_INCOMPLETE;
    }

    if ((VK_F8 == scancode) && !_checkbox_down)
    {
        _checkbox_down = true;
        encui_direct_click_checkbox(encui_find_checkbox(_pages + _id));
    }

    encui_direct_key_textbox(prompt, page, scancode);
    return ENCUI_INCOMPLETE;
}

int
encui_get_page(void)
{
    return _id;
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
            cy += encui_direct_print(cy, buffer) + 1;
        }

        if (ENCUIFT_TEXTBOX == field->type)
        {
            encui_direct_create_textbox((encui_prompt_page *)field->data, page,
                                        &cy);
        }

        if (!has_checkbox && (ENCUIFT_CHECKBOX == field->type))
        {
            has_checkbox = true;
            _checkbox_down = false;
            encui_direct_create_checkbox(field, buffer, sizeof(buffer));
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

    _state = STATE_PROMPT;
    pal_enable_mouse();
    return true;
}
