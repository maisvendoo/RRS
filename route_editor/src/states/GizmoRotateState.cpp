#include "editor/states/GizmoRotateState.h"

GizmoRotateState::GizmoRotateState(EditorContext& editor_context)
    : State(editor_context)
{
    name = "GizmoRotateState";
}

GizmoRotateState::~GizmoRotateState() = default;

void GizmoRotateState::handle_key_press()
{
}
