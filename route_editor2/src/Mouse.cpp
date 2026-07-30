#include "editor/Mouse.h"

#include <vsg/ui/PointerEvent.h>

void Mouse::apply(vsg::ButtonPressEvent& buttonPress)
{
    if (buttonPress.handled)
    {
        return;
    }

    button_mask = buttonPress.mask;
}

void Mouse::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    if (buttonRelease.handled)
    {
        return;
    }

    button_mask = buttonRelease.mask;
}

void Mouse::apply(vsg::MoveEvent& moveEvent)
{
    static int prev_x = moveEvent.x;
    static int prev_y = moveEvent.y;

    x = moveEvent.x;
    y = moveEvent.y;

    delta_x = x - prev_x;
    delta_y = y - prev_y;

    prev_x = x;
    prev_y = y;
}

vsg::ButtonMask Mouse::get_button_mask() const
{
    return button_mask;
}

int Mouse::get_x() const
{
    return x;
}

int Mouse::get_y() const
{
    return y;
}

int Mouse::get_delta_x() const
{
    return delta_x;
}

int Mouse::get_delta_y() const
{
    return delta_y;
}
