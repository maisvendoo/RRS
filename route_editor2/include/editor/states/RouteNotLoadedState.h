#ifndef EDITOR_ROUTE_NOT_LOADED_STATE_H
#define EDITOR_ROUTE_NOT_LOADED_STATE_H

#include "editor/states/EditorState.h"

class RouteNotLoadedState : public EditorState
{
public:
    virtual ~RouteNotLoadedState() override;

    virtual void fill_status_bar() const override;
};

#endif // EDITOR_ROUTE_NOT_LOADED_STATE_H
