#ifndef EDITOR_EVENT_HANDLER_H
#define EDITOR_EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

#include <memory>

class StateManager;

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class KeyPressEvent;
class KeyReleaseEvent;
class MoveEvent;
class ScrollWheelEvent;

} // namespace vsg

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(const std::unique_ptr<StateManager>& state_manager);
    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;
    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;
    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;
    virtual void apply(vsg::MoveEvent& moveEvent) override;
    virtual void apply(vsg::ScrollWheelEvent& scrollWheel) override;

    void update(double delta_time);

private:
    const std::unique_ptr<StateManager>& state_manager;
};

#endif // EDITOR_EVENT_HANDLER_H
