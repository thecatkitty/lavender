#include <algorithm>
#include <cstring>
#include <memory>
#include <vector>

extern "C"
{
#include "../../../resource.h"
#include "direct.h"
}

#include "widgets.hpp"

// Screen metrics
static gfx_dimensions _glyph = {0, 0};
static gfx_rect       _screen = {0, 0, 0, 0};

// Buttons
static gfx_rect _cancel;
static gfx_rect _back;
static gfx_rect _next;
static bool     _checkbox_down;

// Page state
static encui_page *_page;

std::vector<std::unique_ptr<ui::widget>> widgets_{};

void
encui_direct_init_frame(void)
{
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
_is_pressed(const gfx_rect *rect, uint16_t msx, uint16_t msy)
{
    if (0 == rect->width)
    {
        return false;
    }

    msx *= _glyph.width;
    msy *= _glyph.height;

    if ((rect->left > int(msx)) || ((rect->left + rect->width) <= int(msx)))
    {
        return false;
    }

    if ((rect->top > int(msy)) || ((rect->top + rect->height) <= int(msy)))
    {
        return false;
    }

    return true;
}

static void
_create_controls(encui_page *page)
{
    widgets_.clear();
    widgets_.resize(page->length);

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
            auto label = std::make_unique<ui::label>(*page, *field);
            label->move(1, cy);
            label->draw();
            cy = ui::get_bottom(label->get_area());
            widgets_[i] = std::move(label);
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

            auto checkbox = std::make_unique<ui::checkbox>(*page, *field);
            checkbox->move(1, GFX_LINES - 5);
            checkbox->draw();
            widgets_[i] = std::move(checkbox);
        }
    }
}

void
encui_direct_enter_page(encui_page *pages, int id)
{
    _page = pages + id;

    char buffer[GFX_COLUMNS * 2];
    _draw_background();
    pal_load_string(_page->title, buffer, sizeof(buffer));
    _draw_title(buffer);
    _create_controls(_page);

    char caption[9];
    if ((0 < id) && (0 != pages[id - 1].title))
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
}

int
encui_direct_click(uint16_t x, uint16_t y)
{
    if (_is_pressed(&_back, x, y))
    {
        return encui_direct_key(VK_PRIOR);
    }

    if (_is_pressed(&_next, x, y))
    {
        return encui_direct_key(VK_RETURN);
    }

    if (_is_pressed(&_cancel, x, y))
    {
        return encui_direct_key(VK_ESCAPE);
    }

    if (_is_pressed(encui_direct_get_textbox_area(), x, y))
    {
        uint16_t x, y;
        pal_get_mouse(&x, &y);
        encui_direct_click_textbox(encui_find_prompt(_page), _page, x, y);
        return ENCUI_INCOMPLETE;
    }

    auto checkbox_field = encui_find_checkbox(_page);
    if (checkbox_field && !_checkbox_down)
    {
        auto    &checkbox = widgets_[checkbox_field - _page->fields];
        gfx_rect pos = checkbox->get_position();
        pos.left *= _glyph.width;
        pos.width *= _glyph.width;
        pos.top *= _glyph.height;
        pos.height *= _glyph.height;
        if (_is_pressed(&pos, x, y))
        {
            _checkbox_down = true;

            pos = checkbox->get_position();
            checkbox->click(x - pos.left, y - pos.top);
            return ENCUI_INCOMPLETE;
        }
    }

    return ENCUI_INCOMPLETE;
}

int
encui_direct_key(uint16_t scancode)
{
    if (0 == scancode)
    {
        _checkbox_down = false;
        return ENCUI_INCOMPLETE;
    }

    if (VK_ESCAPE == scancode)
    {
        return ENCUI_CANCEL;
    }

    encui_prompt_page *prompt = encui_find_prompt(_page);

    if (VK_RETURN == scancode)
    {
        if (0 >= _page->proc(ENCUIM_CHECK, prompt->buffer, _page->data))
        {
            return ENCUI_OK;
        }

        pal_disable_mouse();
        gfx_draw_rectangle(encui_direct_get_textbox_area(), GFX_COLOR_BLACK);
        encui_direct_animate_textbox(prompt, _page, false);
        return ENCUI_INCOMPLETE;
    }

    int id = encui_get_page();
    if ((VK_PRIOR == scancode) && (0 < id))
    {
        pal_disable_mouse();
        encui_set_page(id - 1);
        return ENCUI_INCOMPLETE;
    }

    if ((VK_F8 == scancode) && !_checkbox_down)
    {
        _checkbox_down = true;

        auto checkbox_field = encui_find_checkbox(_page);
        if (checkbox_field)
        {
            widgets_[checkbox_field - _page->fields]->click(-1, -1);
        }

        return ENCUI_INCOMPLETE;
    }

    encui_direct_key_textbox(prompt, _page, scancode);
    return ENCUI_INCOMPLETE;
}
