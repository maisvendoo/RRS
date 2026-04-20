#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "states/CameraNavigationState.h"
#include "states/InitialState.h"
#include "states/SelectRouteState.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class State;

namespace vsg
{

class KeyPressEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    EventHandler();
    virtual ~EventHandler() override;
    virtual void apply(vsg::KeyPressEvent& keyPress) override;

private:
    State* state = nullptr;
    SelectRouteState select_route_state;
    InitialState initial_state;
    CameraNavigationState camera_navigation_state;
};

#endif // EVENT_HANDLER_H
