#include "editor/states/GizmoScaleState.h"

GizmoScaleState::GizmoScaleState(EditorContext& editor_context)
    : State(editor_context)
{
    name = "GizmoScaleState";
}

GizmoScaleState::~GizmoScaleState() = default;

void GizmoScaleState::handle_key_press()
{
}
