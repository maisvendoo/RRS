#include "Mouse.h"

#include <vsg/ui/PointerEvent.h>
#include <vsg/ui/ScrollWheelEvent.h>

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
    if (moveEvent.handled)
    {
        return;
    }

    static int prev_x = moveEvent.x;
    static int prev_y = moveEvent.y;

    x = moveEvent.x;
    y = moveEvent.y;

    delta_x = x - prev_x;
    delta_y = y - prev_y;

    prev_x = x;
    prev_y = y;
}

void Mouse::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled)
    {
        return;
    }

    scroll = scrollWheel.delta.y;
}
