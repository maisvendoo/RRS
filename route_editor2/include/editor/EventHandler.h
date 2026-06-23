#ifndef EDITOR_EVENT_HANDLER_H
#define EDITOR_EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class StateManager;

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(StateManager& state_manager);

    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;

    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

private:
    StateManager& state_manager;
};

#endif // EDITOR_EVENT_HANDLER_H
