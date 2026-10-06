#ifndef EDITOR_STATES_GIZMO_ROTATE_STATE_H
#define EDITOR_STATES_GIZMO_ROTATE_STATE_H

#include "editor/states/State.h"

class GizmoRotateState : public State
{
public:
    explicit GizmoRotateState(EditorContext& editor_context);
    virtual ~GizmoRotateState() override;

    virtual void handle_key_press() override;
};

#endif // EDITOR_STATES_GIZMO_ROTATE_STATE_H
