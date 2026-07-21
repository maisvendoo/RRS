#include "states/GizmoTranslateState.h"

GizmoTranslateState::GizmoTranslateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

GizmoTranslateState::~GizmoTranslateState() = default;

void GizmoTranslateState::handle_key_press(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;
}
