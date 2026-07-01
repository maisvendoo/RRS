#ifndef EDITOR_MOUSE_H
#define EDITOR_MOUSE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/ui/PointerEvent.h>

class Mouse : public vsg::Inherit<vsg::Visitor, Mouse>
{
public:
    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;

    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;

    vsg::ButtonMask get_button_mask() const;

private:
    vsg::ButtonMask button_mask = vsg::BUTTON_MASK_OFF;
};

#endif // EDITOR_MOUSE_H
