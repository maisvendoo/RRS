#include "states/GizmoScaleState.h"

GizmoScaleState::GizmoScaleState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
}

GizmoScaleState::~GizmoScaleState() = default;

void GizmoScaleState::handle_key_press()
{
}

const char* GizmoScaleState::get_name() const
{
    return "GizmoScaleState";
}
