#include <algorithm>

#include "widgets.hpp"

using namespace ui;

void
panel::draw()
{
    for (auto &child : children_)
    {
        child->draw();
    }
}

int
panel::click(int x, int y)
{
    for (auto &child : children_)
    {
        auto pos = child->get_position();
        if ((pos.left > x) || ((pos.left + pos.width) <= x))
        {
            continue;
        }

        if ((pos.top > y) || ((pos.top + pos.height) <= y))
        {
            continue;
        }

        auto status = child->click(x - pos.left, y - pos.top);
        if (0 != status)
        {
            return status;
        }
    }

    return 0;
}

int
panel::key(int scancode)
{
    for (auto &child : children_)
    {
        auto status = child->key(scancode);
        if (0 != status)
        {
            return status;
        }
    }

    return 0;
}

std::vector<widget_ptr>::iterator
panel::get_child_by_type(int type)
{
    return std::find_if(children_.begin(), children_.end(),
                        [=](const widget_ptr &widget) {
                            return type == widget->get_model().type;
                        });
}

encui_field ui::null_field{};
