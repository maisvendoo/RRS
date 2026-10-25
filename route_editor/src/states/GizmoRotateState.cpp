#include "editor/states/GizmoRotateState.h"

GizmoRotateState::GizmoRotateState(EditorContext& editor_ctx)
    : State(editor_ctx)
{
    name = "GizmoRotateState";
}

GizmoRotateState::~GizmoRotateState() = default;

void GizmoRotateState::handle_key_press()
{
}
