#include "states/KeyboardRotateState.h"

KeyboardRotateState::KeyboardRotateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

KeyboardRotateState::~KeyboardRotateState() = default;

void KeyboardRotateState::handle_key_press(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;
}
