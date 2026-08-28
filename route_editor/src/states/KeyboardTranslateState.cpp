#include "states/KeyboardTranslateState.h"

KeyboardTranslateState::KeyboardTranslateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
    name = "KeyboardTranslateState";
}

KeyboardTranslateState::~KeyboardTranslateState() = default;

void KeyboardTranslateState::handle_key_press()
{
}
