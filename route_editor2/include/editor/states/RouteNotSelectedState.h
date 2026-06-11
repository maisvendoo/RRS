#ifndef EDITOR_ROUTE_NOT_SELECTED_STATE_H
#define EDITOR_ROUTE_NOT_SELECTED_STATE_H

#include "editor/states/EditorState.h"

class RouteNotSelectedState : public EditorState
{
public:
    virtual ~RouteNotSelectedState() override;

    virtual void fill_status_bar() const override;
};

#endif // EDITOR_ROUTE_NOT_SELECTED_STATE_H
