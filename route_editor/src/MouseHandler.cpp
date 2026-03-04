#include "MouseHandler.h"

#include "MouseButton.h"

#include <vsg/maths/vec2.h>
#include <vsg/ui/PointerEvent.h>

void MouseHandler::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    switch (buttonPress.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            is_lmb_pressed = true;
            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            is_mmb_pressed = true;
            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            is_rmb_pressed = true;
            return;
        }
        default:
        {
            return;
        }
    }
}

void MouseHandler::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    switch (buttonRelease.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            is_lmb_pressed = false;
            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            is_mmb_pressed = false;
            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            is_rmb_pressed = false;
            return;
        }
        default:
        {
            return;
        }
    }
}

void MouseHandler::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled)
    {
        return;
    }

    static vsg::ivec2 prev_pos = {moveEvent.x, moveEvent.y};

    pos = {moveEvent.x, moveEvent.y};
    delta_pos = pos - prev_pos;

    prev_pos = pos;
}

vsg::ivec2 MouseHandler::get_pos() const
{
    return pos;
}

vsg::ivec2 MouseHandler::get_delta_pos() const
{
    return delta_pos;
}

bool MouseHandler::get_is_lmb_pressed() const
{
    return is_lmb_pressed;
}

bool MouseHandler::get_is_mmb_pressed() const
{
    return is_mmb_pressed;
}

bool MouseHandler::get_is_rmb_pressed() const
{
    return is_rmb_pressed;
}
