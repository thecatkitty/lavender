#pragma once

#include <memory>
#include <vector>

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

using widget_ptr = std::unique_ptr<widget>;

struct button : widget
{
    button(const encui_page &page, encui_field &field);

    virtual void
    draw() override;

    virtual int
    click(int x, int y) override;
};

struct checkbox : widget
{
    checkbox(const encui_page &page, encui_field &field);

    virtual void
    draw() override;

    virtual int
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

    virtual void
    draw() override;
};

struct panel : widget
{
    panel(const encui_page &page)
        : widget{page, *static_cast<encui_field *>(nullptr)}, children_{}
    {
    }

    virtual void
    draw() override;

    virtual int
    click(int x, int y) override;

    virtual int
    key(int scancode) override;

    void
    append(widget_ptr &&wptr);

    std::vector<widget_ptr>::iterator
    begin()
    {
        return children_.begin();
    }

    std::vector<widget_ptr>::iterator
    end()
    {
        return children_.end();
    }

  private:
    std::vector<widget_ptr> children_;
};

struct textbox : widget
{
    textbox(const encui_page &page, encui_field &field);

    virtual void
    draw() override;

    virtual int
    click(int x, int y) override;

    virtual int
    key(int scancode) override;

    bool
    animate(bool valid);

    void
    alert(char *message);

  private:
    uint32_t blink_start_;
    uint32_t caret_period_;
    uint32_t caret_counter_;
    bool     caret_visible_;
    int      position_;
    int      state_;
};

} // namespace ui
