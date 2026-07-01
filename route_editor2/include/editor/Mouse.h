#ifndef EDITOR_MOUSE_H
#define EDITOR_MOUSE_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;

}

class Mouse : public vsg::Inherit<vsg::Visitor, Mouse>
{
public:
    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;

    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;

    bool is_lmb_pressed() const;
    bool is_mmb_pressed() const;
    bool is_rmb_pressed() const;

private:
    bool lmb_state = false;
    bool mmb_state = false;
    bool rmb_state = false;
};

#endif // EDITOR_MOUSE_H
