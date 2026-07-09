#ifndef EDITOR_EVENT_HANDLER_H
#define EDITOR_EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

namespace vsg
{

class ButtonPressEvent;
class ButtonReleaseEvent;
class ConfigureWindowEvent;
class KeyPressEvent;
class KeyReleaseEvent;
class MoveEvent;

}

class StateManager;

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(StateManager& state_manager);

    virtual ~EventHandler() override;

    virtual void apply(vsg::ConfigureWindowEvent& configureWindow) override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;

    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

    virtual void apply(vsg::ButtonPressEvent& buttonPress) override;

    virtual void apply(vsg::ButtonReleaseEvent& buttonRelease) override;

    virtual void apply(vsg::MoveEvent& moveEvent) override;

private:
    StateManager& state_manager;
};

#endif // EDITOR_EVENT_HANDLER_H
