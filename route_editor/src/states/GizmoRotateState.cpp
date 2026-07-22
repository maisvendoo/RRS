#include "states/GizmoRotateState.h"

GizmoRotateState::GizmoRotateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

GizmoRotateState::~GizmoRotateState() = default;

void GizmoRotateState::handle_key_press()
{
}

const char* GizmoRotateState::get_name() const
{
    return "GizmoRotateState";
}
