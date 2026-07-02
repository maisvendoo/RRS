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

    virtual void apply(vsg::MoveEvent& moveEvent) override;

    vsg::ButtonMask get_button_mask() const;

    int get_pos_x() const;

    int get_pos_y() const;

    int get_delta_x() const;

    int get_delta_y() const;

private:
    vsg::ButtonMask button_mask = vsg::BUTTON_MASK_OFF;
    int pos_x;
    int pos_y;
    int delta_x;
    int delta_y;
};

#endif // EDITOR_MOUSE_H
