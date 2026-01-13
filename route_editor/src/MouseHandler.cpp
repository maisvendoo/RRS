#include "MouseHandler.h"

#include "MouseButton.h"

#include <vsg/maths/vec2.h>
#include <vsg/ui/ApplicationEvent.h>
#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>

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
    }
}

void MouseHandler::apply(vsg::MoveEvent& moveEvent)
{
    if (moveEvent.handled)
    {
        return;
    }

    static vsg::ivec2 prev_mouse_pos = {moveEvent.x, moveEvent.y};

    mouse_pos = {moveEvent.x, moveEvent.y};
    delta_pos = mouse_pos - prev_mouse_pos;
    used_delta_pos = false;

    prev_mouse_pos = mouse_pos;
}

void MouseHandler::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled)
    {
        return;
    }

    scroll += scrollWheel.delta.y;
    used_scroll = false;
}

void MouseHandler::apply(vsg::FrameEvent& frame)
{
    (void)frame;

    if (used_delta_pos)
    {
        delta_pos = {0, 0};
    }

    if (used_scroll)
    {
        scroll = 0.0f;
    }
}

vsg::ivec2 MouseHandler::get_mouse_pos() const
{
    return mouse_pos;
}

vsg::ivec2 MouseHandler::get_delta_pos()
{
    used_delta_pos = true;
    return delta_pos;
}

float MouseHandler::get_scroll()
{
    used_scroll = true;
    return scroll;
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
