#include "states/KeyboardScaleState.h"

KeyboardScaleState::KeyboardScaleState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

KeyboardScaleState::~KeyboardScaleState() = default;

void KeyboardScaleState::handle_key_press()
{
}

const char* KeyboardScaleState::get_name() const
{
    return "KeyboardScaleState";
}
