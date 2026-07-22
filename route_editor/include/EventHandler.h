#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class StateManager;

namespace vsg
{

class ButtonPressEvent;
class KeyPressEvent;
class KeyReleaseEvent;
class MoveEvent;
class ScrollWheelEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(StateManager& state_manager);
    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;
    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;
    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    virtual void apply(vsg::MoveEvent& moveEvent) override;
    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;

    void update(double delta_time);

private:
    StateManager& state_manager;
};

#endif // EVENT_HANDLER_H
