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

vsg::ButtonMask Mouse::get_button_mask() const
{
    return button_mask;
}
