#ifndef MOUSE_HANDLER_H
#define MOUSE_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/maths/vec2.h>

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class FrameEvent;
class MoveEvent;
class ScrollWheelEvent;

}

class MouseHandler : public vsg::Inherit<vsg::Visitor, MouseHandler>
{
public:
    void apply(vsg::ButtonPressEvent& buttonPress) override;
    void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    void apply(vsg::MoveEvent& moveEvent) override;
    void apply(vsg::ScrollWheelEvent& scrollWheel) override;
    void apply(vsg::FrameEvent& frame) override;

    vsg::ivec2 get_mouse_pos() const;
    vsg::ivec2 get_delta_pos();
    float get_scroll();

    bool get_is_lmb_pressed() const;
    bool get_is_mmb_pressed() const;
    bool get_is_rmb_pressed() const;

private:
    vsg::ivec2 mouse_pos = {0, 0};
    vsg::ivec2 delta_pos = {0, 0};
    bool used_delta_pos = false;

    float scroll = 0.0f;
    bool used_scroll = false;

    bool is_lmb_pressed = false;
    bool is_mmb_pressed = false;
    bool is_rmb_pressed = false;
};

#endif // MOUSE_HANDLER_H
