#include "editor/Mouse.h"

#include <vsg/ui/PointerEvent.h>

void Mouse::apply(vsg::ButtonPressEvent& buttonPress)
{
    button_mask = buttonPress.mask;
}

void Mouse::apply(vsg::ButtonReleaseEvent& buttonRelease)
{
    button_mask = buttonRelease.mask;
}

vsg::ButtonMask Mouse::get_button_mask() const
{
    return button_mask;
}
