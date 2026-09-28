#ifndef EDITOR_MOUSE_H
#define EDITOR_MOUSE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/ui/PointerEvent.h>

class Mouse : public vsg::Inherit<vsg::Visitor, Mouse>
{
public:
    virtual ~Mouse() = default;

    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;
    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    virtual void apply(vsg::MoveEvent& moveEvent) override;
    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;

    vsg::ButtonMask get_button_mask() const { return button_mask; }
    bool is_lmb_pressed() const { return button_mask & vsg::BUTTON_MASK_1; }
    bool is_mmb_pressed() const { return button_mask & vsg::BUTTON_MASK_2; }
    bool is_rmb_pressed() const { return button_mask & vsg::BUTTON_MASK_3; }
    int get_x() const { return x; }
    int get_y() const { return y; }
    int get_delta_x() const { return delta_x; }
    int get_delta_y() const { return delta_y; }
    float get_scroll() const { return scroll; }

private:
    vsg::ButtonMask button_mask;
    int x;
    int y;
    int delta_x;
    int delta_y;
    float scroll;
};

#endif // EDITOR_MOUSE_H
