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

    pos_x = moveEvent.x;
    pos_y = moveEvent.y;

    delta_x = pos_x - prev_x;
    delta_y = pos_y - prev_y;

    prev_x = pos_x;
    prev_y = pos_y;
}

void Mouse::apply(vsg::ScrollWheelEvent& scrollWheel)
{
    if (scrollWheel.handled)
    {
        return;
    }

    scroll = scrollWheel.delta.y;
}

int Mouse::get_pos_x() const
{
    return pos_x;
}
int Mouse::get_pos_y() const
{
    return pos_y;
}
int Mouse::get_delta_x() const
{
    return delta_x;
}
int Mouse::get_delta_y() const
{
    return delta_y;
}

std::uint16_t Mouse::get_button_mask() const
{
    return button_mask;
}

bool Mouse::is_lmb_pressed() const
{
    return button_mask & vsg::BUTTON_MASK_1;
}

bool Mouse::is_mmb_pressed() const
{
    return button_mask & vsg::BUTTON_MASK_2;
}

bool Mouse::is_rmb_pressed() const
{
    return button_mask & vsg::BUTTON_MASK_3;
}
