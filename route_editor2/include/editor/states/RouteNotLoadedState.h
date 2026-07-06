#ifndef EDITOR_ROUTE_NOT_LOADED_STATE_H
#define EDITOR_ROUTE_NOT_LOADED_STATE_H

#include "editor/states/EditorState.h"

#include <vsg/core/ref_ptr.h>

class Keyboard;
class Mouse;
class StateManager;

class RouteNotLoadedState : public EditorState
{
public:
    RouteNotLoadedState(
        const vsg::ref_ptr<const Mouse>& mouse,
        const vsg::ref_ptr<const Keyboard>& keyboard,
        StateManager& state_manager
    );

    virtual ~RouteNotLoadedState() override;

    virtual void fill_status_bar() const override;
};

#endif // EDITOR_ROUTE_NOT_LOADED_STATE_H
