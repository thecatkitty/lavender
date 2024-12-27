#pragma once

#include <memory>
#include <vector>

extern "C"
{
#include "direct.h"
}

namespace ui
{

extern encui_field null_field;

inline int
get_bottom(gfx_rect rect)
{
    return rect.top + rect.height;
}

struct widget
{
    widget(const widget &) = delete;
    virtual ~widget() = default;

    widget(const encui_page &page)
        : page_{&page}, field_{null_field}, rect_{}, parent_{nullptr}
    {
    }

    widget(encui_field &field)
        : page_{}, field_{field}, rect_{}, parent_{nullptr}
    {
    }

    encui_field
    get_model() const
    {
        return field_;
    }

    const encui_page *
    get_page() const
    {
        return parent_ ? parent_->get_page() : page_;
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
    const encui_page *page_;
    encui_field      &field_;
    gfx_rect          rect_;
    widget           *parent_;
};

using widget_ptr = std::unique_ptr<widget>;

struct button : widget
{
    button(encui_field &field);

    void
    draw() override;

    int
    click(int x, int y) override;
};

struct checkbox : widget
{
    checkbox(encui_field &field);

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
    label(encui_field &field) : widget{field}
    {
    }

    void
    draw() override;
};

struct panel : widget
{
    panel(const encui_page &page) : widget{page}, children_{}
    {
    }

    void
    draw() override;

    int
    click(int x, int y) override;

    int
    key(int scancode) override;

    template <typename T, typename... Args>
    T &
    create(Args &&...args)
    {
        auto child = std::make_unique<T>(std::forward<Args>(args)...);
        child->set_parent(this);
        return *reinterpret_cast<T *>(
            children_.insert(children_.end(), std::move(child))->get());
    }

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

    std::vector<widget_ptr>::iterator
    get_child_by_type(int type, size_t nth = 0);

  private:
    std::vector<widget_ptr> children_;
};

struct textbox : widget
{
    textbox(encui_field &field);

    void
    draw() override;

    int
    click(int x, int y) override;

    int
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

template <typename T> struct field_type
{
    static const int type = ENCUIFT_SEPARATOR;
};
template <> struct field_type<checkbox>
{
    static const int type = ENCUIFT_CHECKBOX;
};

template <> struct field_type<label>
{
    static const int type = ENCUIFT_LABEL;
};

template <> struct field_type<textbox>
{
    static const int type = ENCUIFT_TEXTBOX;
};

template <typename T>
T *
get_child(panel &panel, size_t nth = 0)
{
    auto it = panel.get_child_by_type(field_type<T>::type, nth);
    return (panel.end() == it) ? nullptr : reinterpret_cast<T *>(it->get());
}

} // namespace ui
