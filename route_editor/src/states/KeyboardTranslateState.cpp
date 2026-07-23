#include "states/KeyboardTranslateState.h"

KeyboardTranslateState::KeyboardTranslateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
}

KeyboardTranslateState::~KeyboardTranslateState() = default;

void KeyboardTranslateState::handle_key_press()
{
}

const char* KeyboardTranslateState::get_name() const
{
    return "KeyboardTranslateState";
}
