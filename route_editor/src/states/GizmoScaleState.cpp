#include "states/GizmoScaleState.h"

GizmoScaleState::GizmoScaleState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
    name = "GizmoScaleState";
}

GizmoScaleState::~GizmoScaleState() = default;

void GizmoScaleState::handle_key_press()
{
}
