#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/maths/vec2.h>

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class MoveEvent;

}

class MouseHandler : public vsg::Inherit<vsg::Visitor, MouseHandler>
{
public:
    virtual ~MouseHandler() = default;

    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;
    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    virtual void apply(vsg::MoveEvent& moveEvent) override;

    vsg::ivec2 get_pos() const;
    // Must be called only from MoveEvents
    vsg::ivec2 get_delta_pos() const;

    bool get_is_lmb_pressed() const;
    bool get_is_mmb_pressed() const;
    bool get_is_rmb_pressed() const;

private:
    vsg::ivec2 pos = {0, 0};
    vsg::ivec2 delta_pos = {0, 0};

    bool is_lmb_pressed = false;
    bool is_mmb_pressed = false;
    bool is_rmb_pressed = false;
};

#endif // MOUSE_HANDLER_H
