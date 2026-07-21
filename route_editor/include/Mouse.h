#ifndef MOUSE_H
#define MOUSE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class MoveEvent;
class ScrollWheelEvent;

}

class Mouse : public vsg::Inherit<vsg::Visitor, Mouse>
{
public:
    virtual ~Mouse() = default;

    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;
    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    virtual void apply(vsg::MoveEvent& moveEvent) override;
    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;

    int get_pos_x() const;
    int get_pos_y() const;
    // Must be called only from MoveEvents
    int get_delta_x() const;
    int get_delta_y() const;

    bool is_lmb_pressed() const;
    bool is_mmb_pressed() const;
    bool is_rmb_pressed() const;

private:
    int pos_x;
    int pos_y;
    int delta_x;
    int delta_y;
    int scroll;

    bool is_lmb_pressed_ = false;
    bool is_mmb_pressed_ = false;
    bool is_rmb_pressed_ = false;
};

#endif // MOUSE_H
