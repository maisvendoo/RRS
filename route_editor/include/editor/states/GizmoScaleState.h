#ifndef GIZMO_SCALE_STATE_H
#define GIZMO_SCALE_STATE_H

#include "editor/states/State.h"

class GizmoScaleState : public State
{
public:
    GizmoScaleState(EditorContext& editor_context);
    virtual ~GizmoScaleState() override;

    virtual void handle_key_press() override;
};

#endif // GIZMO_SCALE_STATE_H
