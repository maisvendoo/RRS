#include "states/KeyboardRotateState.h"

KeyboardRotateState::KeyboardRotateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
}

KeyboardRotateState::~KeyboardRotateState() = default;

void KeyboardRotateState::handle_key_press()
{
}

const char* KeyboardRotateState::get_name() const
{
    return "KeyboardRotateState";
}
