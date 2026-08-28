#include "states/KeyboardScaleState.h"

KeyboardScaleState::KeyboardScaleState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard,
    StateManager& state_manager
)
    : State(mouse, keyboard, state_manager)
{
    name = "KeyboardScaleState";
}

KeyboardScaleState::~KeyboardScaleState() = default;

void KeyboardScaleState::handle_key_press()
{
}
