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
    State* state_;
    SelectRouteState select_route_state_;
    InitialState initial_state_;
    CameraNavigationState camera_navigation_state_;
};

#endif // EVENT_HANDLER_H
