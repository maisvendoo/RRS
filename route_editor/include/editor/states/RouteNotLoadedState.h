#ifndef ROUTE_NOT_LOADED_STATE_H
#define ROUTE_NOT_LOADED_STATE_H

#include "editor/states/State.h"

class RouteNotLoadedState : public State
{
public:
    RouteNotLoadedState(EditorContext& editor_context);
    virtual ~RouteNotLoadedState() override;

    virtual void draw_gui() const override;

    virtual void fill_status_bar() const override;
};

#endif // ROUTE_NOT_LOADED_STATE_H
