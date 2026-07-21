#include "states/GizmoScaleState.h"

GizmoScaleState::GizmoScaleState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

GizmoScaleState::~GizmoScaleState() = default;

void GizmoScaleState::handle_key_press()
{
}
