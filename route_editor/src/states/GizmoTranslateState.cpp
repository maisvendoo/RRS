#include "states/GizmoTranslateState.h"

GizmoTranslateState::GizmoTranslateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
    name = "GizmoTranslateState";
}

GizmoTranslateState::~GizmoTranslateState() = default;

void GizmoTranslateState::handle_key_press()
{
}
