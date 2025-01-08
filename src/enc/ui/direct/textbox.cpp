#include <algorithm>
#include <cstring>

#include "widgets.hpp"

using namespace ui;

namespace
{
gfx_rect
get_caret(const gfx_rect &rect, int position)
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);
    return gfx_rect{(rect.left + position + 1) * glyph.width,
                    (rect.top + 1) * glyph.height, 1, glyph.height};
}

gfx_rect
get_field(const gfx_rect &rect)
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);
    return gfx_rect{rect.left * glyph.width,
                    (rect.top + 1) * glyph.height - (glyph.width / 2),
                    rect.width * glyph.width, glyph.height + glyph.width};
}
} // namespace

enum
{
    STATE_PROMPT,
    STATE_INVALID1,
    STATE_INVALID2,
    STATE_INVALID3,
};

textbox::textbox(encui_field &field)
    : widget{field}, blink_start_{}, caret_period_{}, caret_counter_{},
      caret_visible_{true}, position_{}, state_{STATE_PROMPT}
{
    auto &textbox = *reinterpret_cast<encui_textbox_data *>(field.data);
    if (0 == textbox.length)
    {
        textbox.buffer[0] = 0;
    }

    size_t field_width = GFX_COLUMNS / 2 - 1;
    if (field_width < textbox.capacity)
    {
        field_width = textbox.capacity;
    }

    position_ = textbox.length;

    rect_.width = field_width + 2;
    rect_.height = 5;
}

void
textbox::draw()
{
    auto &page = *get_page();
    auto &textbox = *reinterpret_cast<encui_textbox_data *>(field_.data);

    auto pos = get_position();
    auto field = get_field(pos);
    gfx_draw_rectangle(&field, (0 < encui_check_page(&page, textbox.buffer))
                                   ? GFX_COLOR_GRAY
                                   : GFX_COLOR_BLACK);
    gfx_fill_rectangle(&field, GFX_COLOR_WHITE);
    gfx_draw_text(textbox.buffer, rect_.left + 1, rect_.top + 1);

    caret_period_ = palpp_get_ticks(500);
    caret_counter_ = palpp_get_counter();
}

bool
textbox::animate(bool valid)
{
    if (!valid && (STATE_PROMPT == state_))
    {
        state_ = STATE_INVALID1;
        blink_start_ = palpp_get_counter();
        return false;
    }

    auto pos = get_position();
    auto field = get_field(pos);
    if (STATE_INVALID1 == state_)
    {
        if (palpp_get_counter() > blink_start_ + palpp_get_ticks(63))
        {
            gfx_draw_rectangle(&field, GFX_COLOR_GRAY);
            state_ = STATE_INVALID2;
        }

        return false;
    }

    if (STATE_INVALID2 == state_)
    {
        if (palpp_get_counter() > blink_start_ + palpp_get_ticks(126))
        {
            gfx_draw_rectangle(&field, GFX_COLOR_BLACK);
            state_ = STATE_INVALID3;
        }

        return false;
    }

    if (STATE_INVALID3 == state_)
    {
        if (palpp_get_counter() > blink_start_ + palpp_get_ticks(189))
        {
            gfx_draw_rectangle(&field, GFX_COLOR_GRAY);
            draw();
            state_ = STATE_PROMPT;
            pal_enable_mouse();
        }

        return false;
    }

    if (palpp_get_counter() > caret_counter_ + caret_period_)
    {
        auto pos = get_position();
        auto caret = get_caret(pos, position_);
        gfx_draw_line(&caret,
                      caret_visible_ ? GFX_COLOR_BLACK : GFX_COLOR_WHITE);
        caret_counter_ = palpp_get_counter();
        caret_visible_ = !caret_visible_;
    }

    return true;
}

void
textbox::alert(char *message)
{
    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    gfx_rect bg = {0, (rect_.top + 3) * glyph.height, rect_.width * glyph.width,
                   3 * glyph.height};
    gfx_fill_rectangle(&bg, GFX_COLOR_WHITE);

    gfx_rect pos = get_position();
    encui_direct_print(pos.top + 3, message);
}

int
textbox::click(int x, int y)
{
    if (2 < y)
    {
        return 0;
    }

    auto &textbox = *reinterpret_cast<encui_textbox_data *>(field_.data);

    int cursor = std::max(0, std::min(int(textbox.length), x - 1));
    if (position_ != cursor)
    {
        position_ = cursor;
        pal_disable_mouse();
        draw();
        pal_enable_mouse();
    }

    return 0;
}

int
textbox::key(int scancode)
{
    auto &textbox = *reinterpret_cast<encui_textbox_data *>(field_.data);

    pal_disable_mouse();

    gfx_dimensions glyph;
    gfx_get_glyph_dimensions(&glyph);

    auto pos = get_position();
    auto caret = get_caret(pos, position_);
    gfx_draw_line(&caret, GFX_COLOR_WHITE);

    if ((VK_LEFT == scancode) && (0 < position_))
    {
        position_--;
        draw();
    }

    if ((VK_RIGHT == scancode) && (int(textbox.length) > position_))
    {
        position_++;
        draw();
    }

    if ((VK_BACK == scancode) && (0 < position_))
    {
        std::memmove(textbox.buffer + position_ - 1, textbox.buffer + position_,
                     textbox.length - position_);
        position_--;
        textbox.length--;
        textbox.buffer[textbox.length] = 0;
        draw();
    }

    if ((VK_DELETE == scancode) && (int(textbox.length) > position_))
    {
        std::memmove(textbox.buffer + position_, textbox.buffer + position_ + 1,
                     textbox.length - position_ - 1);
        textbox.length--;
        textbox.buffer[textbox.length] = 0;
        draw();
    }

    if (((' ' == scancode) || (VK_OEM_MINUS == scancode) ||
         ((VK_DELETE < scancode) && (VK_F1 > scancode))) &&
        (textbox.length < textbox.capacity))
    {
        std::memmove(textbox.buffer + position_ + 1, textbox.buffer + position_,
                     textbox.length - position_);
        textbox.buffer[position_] =
            (VK_OEM_MINUS == scancode) ? '-' : (scancode & 0xFF);
        position_++;
        textbox.length++;
        textbox.buffer[textbox.length] = 0;
        draw();
    }

    caret = get_caret(pos, position_);
    gfx_draw_line(&caret, GFX_COLOR_BLACK);
    pal_enable_mouse();
    return 0;
}
