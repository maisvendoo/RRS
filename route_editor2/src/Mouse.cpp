#include "editor/Mouse.h"

#include <vsg/ui/PointerEvent.h>

enum MouseButton
{
    MOUSE_BUTTON_LEFT = 1,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT
};

void Mouse::apply(vsg::ButtonPressEvent& buttonPress)
{
    switch (buttonPress.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            lmb_state = true;
            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            mmb_state = true;
            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            rmb_state = true;
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
    switch (buttonRelease.button)
    {
        case MOUSE_BUTTON_LEFT:
        {
            lmb_state = false;
            return;
        }
        case MOUSE_BUTTON_MIDDLE:
        {
            mmb_state = false;
            return;
        }
        case MOUSE_BUTTON_RIGHT:
        {
            rmb_state = false;
            return;
        }
        default:
        {
            return;
        }
    }
}


bool Mouse::is_lmb_pressed() const
{
    return lmb_state;
}

bool Mouse::is_mmb_pressed() const
{
    return mmb_state;
}

bool Mouse::is_rmb_pressed() const
{
    return rmb_state;
}

