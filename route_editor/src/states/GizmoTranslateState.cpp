#include "editor/states/GizmoTranslateState.h"

GizmoTranslateState::GizmoTranslateState(EditorContext& editor_context)
    : State(editor_context)
{
    name = "GizmoTranslateState";
}

GizmoTranslateState::~GizmoTranslateState() = default;

void GizmoTranslateState::handle_key_press()
{
}
