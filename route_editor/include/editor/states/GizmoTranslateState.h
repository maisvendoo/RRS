#ifndef EDITOR_STATES_GIZMO_TRANSLATE_STATE_H
#define EDITOR_STATES_GIZMO_TRANSLATE_STATE_H

#include "editor/states/State.h"

class GizmoTranslateState : public State
{
public:
    GizmoTranslateState(EditorContext& editor_context);
    virtual ~GizmoTranslateState() override;

    virtual void handle_key_press() override;
};

#endif // EDITOR_STATES_GIZMO_TRANSLATE_STATE_H
