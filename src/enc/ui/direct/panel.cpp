#include "widgets.hpp"

using namespace ui;

void
panel::append(widget_ptr &&wptr)
{
    wptr->set_parent(this);
    children_.push_back(std::move(wptr));
}

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
