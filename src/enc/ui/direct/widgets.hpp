#pragma once

extern "C"
{
#include "direct.h"
}

namespace ui
{

inline int
get_bottom(gfx_rect rect)
{
    return rect.top + rect.height;
}

struct widget
{
    widget(const widget &) = delete;
    virtual ~widget() = default;

    widget(const encui_page &page, encui_field &field)
        : page_{page}, field_{field}, rect_{}, parent_{nullptr}
    {
    }

    encui_field
    get_model() const
    {
        return field_;
    }

    void
    move(int left, int top)
    {
        rect_.left = left;
        rect_.top = top;
    }

    const gfx_rect &
    get_area() const
    {
        return rect_;
    }

    gfx_rect
    get_position() const
    {
        return parent_ ? gfx_rect{parent_->rect_.left + rect_.left,
                                  parent_->rect_.top + rect_.top, rect_.width,
                                  rect_.height}
                       : rect_;
    }

    void
    set_parent(widget *parent)
    {
        parent_ = parent;
    }

    virtual void
    draw()
    {
    }

    virtual int
    click(int x, int y)
    {
        return 0;
    }

    virtual int
    key(int scancode)
    {
        return 0;
    }

  protected:
    const encui_page &page_;
    encui_field      &field_;
    gfx_rect          rect_;
    widget           *parent_;
};

struct checkbox : widget
{
    checkbox(const encui_page &page, encui_field &field);

    void
    draw() override;

    int
    click(int x, int y) override;

  private:
    void
    mark(bool checked);

    gfx_rect box_;
};

struct label : widget
{
    label(const encui_page &page, encui_field &field) : widget{page, field}
    {
    }

    void
    draw() override;
};

} // namespace ui
