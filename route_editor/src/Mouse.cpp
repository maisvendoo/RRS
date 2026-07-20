#include "Mouse.h"

#include "MouseButton.h"

#include <vsg/maths/vec2.h>
#include <vsg/ui/PointerEvent.h>

void Mouse::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    switch (buttonPress.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            is_lmb_pressed_ = true;
            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            is_mmb_pressed_ = true;
            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            is_rmb_pressed_ = true;
            return;
        }
        default:
        {
            return;
        }
    }
}

void Mouse::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    switch (buttonRelease.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            is_lmb_pressed_ = false;
            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            is_mmb_pressed_ = false;
            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            is_rmb_pressed_ = false;
            return;
        }
        default:
        {
            return;
        }
    }
}

void Mouse::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled)
    {
        return;
    }

    static vsg::ivec2 prev_pos = {moveEvent.x, moveEvent.y};

    pos_ = {moveEvent.x, moveEvent.y};
    delta_pos_ = pos_ - prev_pos;

    prev_pos = pos_;
}

vsg::ivec2 Mouse::get_pos() const
{
    return pos_;
}

vsg::ivec2 Mouse::get_delta_pos() const
{
    return delta_pos_;
}

bool Mouse::is_lmb_pressed() const
{
    return is_lmb_pressed_;
}

bool Mouse::is_mmb_pressed() const
{
    return is_mmb_pressed_;
}

bool Mouse::is_rmb_pressed() const
{
    return is_rmb_pressed_;
}
