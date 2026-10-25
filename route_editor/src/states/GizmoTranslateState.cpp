#include "editor/states/GizmoTranslateState.h"

GizmoTranslateState::GizmoTranslateState(EditorContext& editor_ctx)
    : State(editor_ctx)
{
    name = "GizmoTranslateState";
}

GizmoTranslateState::~GizmoTranslateState() = default;

void GizmoTranslateState::handle_key_press()
{
}
