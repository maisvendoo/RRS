#include "states/GizmoRotateState.h"

GizmoRotateState::GizmoRotateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
    name = "GizmoRotateState";
}

GizmoRotateState::~GizmoRotateState() = default;

void GizmoRotateState::handle_key_press()
{
}
