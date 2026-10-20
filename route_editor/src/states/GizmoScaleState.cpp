#include "editor/states/GizmoScaleState.h"

GizmoScaleState::GizmoScaleState(EditorContext& editor_ctx)
    : State(editor_ctx)
{
    name = "GizmoScaleState";
}

GizmoScaleState::~GizmoScaleState() = default;

void GizmoScaleState::handle_key_press()
{
}
