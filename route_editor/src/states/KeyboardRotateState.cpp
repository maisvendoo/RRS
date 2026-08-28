#include "states/KeyboardRotateState.h"

KeyboardRotateState::KeyboardRotateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
    name = "KeyboardRotateState";
}

KeyboardRotateState::~KeyboardRotateState() = default;

void KeyboardRotateState::handle_key_press()
{
}
