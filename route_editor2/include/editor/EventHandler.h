#ifndef EDITOR_EVENT_HANDLER_H
#define EDITOR_EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/core/ref_ptr.h>

class Keyboard;
class StateManager;

namespace vsg
{

class FocusInEvent;
class FocusOutEvent;
class KeyPressEvent;
class KeyReleaseEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(StateManager& state_manager);

    ~EventHandler();

    virtual void apply(vsg::KeyPressEvent& keyPress) override;

    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

    virtual void apply(vsg::FocusInEvent& focusIn) override;

    virtual void apply(vsg::FocusOutEvent& focusOut) override;

private:
    StateManager& state_manager;

    vsg::ref_ptr<Keyboard> keyboard;
};

#endif // EDITOR_EVENT_HANDLER_H
