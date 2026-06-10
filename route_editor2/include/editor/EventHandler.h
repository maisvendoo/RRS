#ifndef EDITOR_EVENT_HANDLER_H
#define EDITOR_EVENT_HANDLER_H

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class StateManager;

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(StateManager& state_manager);

private:
    StateManager& state_manager;
};

#endif // EDITOR_EVENT_HANDLER_H
