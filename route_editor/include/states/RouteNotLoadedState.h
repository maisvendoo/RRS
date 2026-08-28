#ifndef ROUTE_NOT_LOADED_STATE_H
#define ROUTE_NOT_LOADED_STATE_H

#include "states/State.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;
class StateManager;

class RouteNotLoadedState : public State
{
public:
    RouteNotLoadedState(
        const vsg::ref_ptr<Mouse>& mouse,
        const vsg::ref_ptr<Keyboard>& keyboard,
        StateManager& state_manager
    );

    virtual ~RouteNotLoadedState() override;

    virtual void fill_status_bar() const override;
};

#endif // ROUTE_NOT_LOADED_STATE_H
