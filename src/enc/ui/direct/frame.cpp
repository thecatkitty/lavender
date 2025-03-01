#include <algorithm>
#include <cstring>
#include <iterator>
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
static encui_field _cancel_field{0, ENCUIFF_STATIC, IDS_CANCEL};
static encui_field _back_field{0, ENCUIFF_STATIC, IDS_BACK};
static encui_field _next_field{0, ENCUIFF_STATIC, IDS_NEXT};
static ui::button  _cancel{_cancel_field};
static ui::button  _back{_back_field};
static ui::button  _next{_next_field};
static bool        _mouse_down;

// Page state
static encui_page *_page;

std::unique_ptr<ui::panel> panel_{};

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
    gfx_draw_text("(C) 2021-2025 Mateusz Karcz", 1, 24);

    _next.move(GFX_COLUMNS - 22, GFX_LINES - 3);
    _cancel.move(GFX_COLUMNS - 11, GFX_LINES - 3);
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

static bool
_is_pressed(const ui::widget &widget, uint16_t msx, uint16_t msy)
{
    gfx_rect pos = widget.get_position();
    if (0 > pos.left)
    {
        return false;
    }

    if ((pos.left > int(msx)) || ((pos.left + pos.width) <= int(msx)))
    {
        return false;
    }

    if ((pos.top > int(msy)) || ((pos.top + pos.height) <= int(msy)))
    {
        return false;
    }

    return true;
}

static void
_create_controls(encui_page *page)
{
    panel_ = std::make_unique<ui::panel>(*page);

    uint16_t x, y;
    _mouse_down = PAL_MOUSE_LBUTTON & pal_get_mouse(&x, &y);

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
            auto &label = panel_->create<ui::label>(*field);
            if (ENCUIFF_FOOTER & field->flags)
            {
                label.move(1, GFX_LINES - 5);
                label.draw();
            }
            else
            {
                label.move(1, cy);
                label.draw();
                cy = ui::get_bottom(label.get_area());
            }
        }

        if (ENCUIFT_TEXTBOX == field->type)
        {
            auto &textbox = panel_->create<ui::textbox>(*field);
            textbox.move(1, cy);
            textbox.draw();
            cy = ui::get_bottom(textbox.get_area());
        }

        if (!has_checkbox && (ENCUIFT_CHECKBOX == field->type))
        {
            has_checkbox = true;

            auto &checkbox = panel_->create<ui::checkbox>(*field);
            checkbox.move(1, GFX_LINES - 5);
            checkbox.draw();
        }

        if (ENCUIFT_OPTION == field->type)
        {
            auto &option = panel_->create<ui::option>(*field);
            option.move(1, cy);
            option.draw();
            cy = ui::get_bottom(option.get_area());
        }

        if (ENCUIFT_BITMAP == field->type)
        {
            auto &bitmap = panel_->create<ui::bitmap>(*field);
            bitmap.move(1, cy);
            bitmap.draw();
            cy = ui::get_bottom(bitmap.get_area());
        }
    }
}

void
encui_direct_enter_page(encui_page *pages, int id)
{
    _page = pages + id;
    _page->proc(ENCUIM_INIT, NULL, _page->data);

    char buffer[GFX_COLUMNS * 2];
    _draw_background();
    pal_load_string(_page->title, buffer, sizeof(buffer));
    _draw_title(buffer);
    _create_controls(_page);

    if (0 < id)
    {
        _back.move(GFX_COLUMNS - 32, GFX_LINES - 3);
        _back.draw();
    }
    else
    {
        _back.move(-1, -1);
    }

    _next.draw();
    _cancel.draw();

    _page->proc(ENCUIM_ENTERED, NULL, _page->data);
    pal_enable_mouse();
}

int
encui_direct_click(uint16_t x, uint16_t y)
{
    if (_mouse_down)
    {
        return ENCUI_INCOMPLETE;
    }

    _mouse_down = true;
    if (_is_pressed(_back, x, y))
    {
        return encui_direct_key(VK_PRIOR);
    }

    if (_is_pressed(_next, x, y))
    {
        return encui_direct_key(VK_RETURN);
    }

    if (_is_pressed(_cancel, x, y))
    {
        return encui_direct_key(VK_ESCAPE);
    }

    auto pos = panel_->get_position();
    panel_->click(x - pos.left, y - pos.top);
    return ENCUI_INCOMPLETE;
}

int
encui_direct_key(uint16_t scancode)
{
    _mouse_down = false;
    if (0 == scancode)
    {
        return ENCUI_INCOMPLETE;
    }

    if (VK_ESCAPE == scancode)
    {
        return ENCUI_CANCEL;
    }

    encui_textbox_data *textbox = encui_find_textbox(_page);

    if (VK_RETURN == scancode)
    {
        if (NULL == textbox)
        {
            return ENCUI_OK;
        }

        if (0 >= encui_check_page(_page, textbox->buffer))
        {
            return ENCUI_OK;
        }

        pal_disable_mouse();
        encui_direct_animate(false);
        return ENCUI_INCOMPLETE;
    }

    int id = encui_get_page();
    if ((VK_PRIOR == scancode) && (0 < id))
    {
        pal_disable_mouse();
        encui_set_page(id - 1);
        return ENCUI_INCOMPLETE;
    }

    if ((VK_F1 <= scancode) && (VK_F7 >= scancode))
    {
        auto option = ui::get_child<ui::option>(*panel_, scancode - VK_F1);
        if (option)
        {
            option->click(-1, -1);
        }

        return ENCUI_INCOMPLETE;
    }

    if (VK_F8 == scancode)
    {
        auto checkbox = ui::get_child<ui::checkbox>(*panel_);
        if (checkbox)
        {
            checkbox->click(-1, -1);
        }

        return ENCUI_INCOMPLETE;
    }

    panel_->key(scancode);
    return ENCUI_INCOMPLETE;
}

bool
encui_direct_animate(bool valid)
{
    auto textbox = ui::get_child<ui::textbox>(*panel_);
    return textbox ? textbox->animate(valid) : true;
}

void
encui_direct_set_error(char *message)
{
    auto textbox = ui::get_child<ui::textbox>(*panel_);
    if (textbox)
    {
        textbox->alert(message);
    }
}

bool
encui_refresh_field(encui_page *page, int id)
{
    if (_page != page)
    {
        return false;
    }

    auto it = panel_->begin() + id;
    (*it)->draw();
    return true;
}
