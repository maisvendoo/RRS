#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include "states/CameraNavigationState.h"
#include "states/InitialState.h"
#include "states/SelectRouteState.h"

#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>

class Keyboard;
class State;

namespace vsg
{

class KeyPressEvent;
class KeyReleaseEvent;

}

class EventHandler : public vsg::Inherit<vsg::Visitor, EventHandler>
{
public:
    explicit EventHandler(Keyboard* keyboard);
    virtual ~EventHandler() override;

    virtual void apply(vsg::KeyPressEvent& keyPress) override;
    virtual void apply(vsg::KeyReleaseEvent& keyRelease) override;

private:
    Keyboard* const keyboard_;

    State* state_;
    SelectRouteState select_route_state_;
    InitialState initial_state_;
    CameraNavigationState camera_navigation_state_;
};

#endif // EVENT_HANDLER_H
