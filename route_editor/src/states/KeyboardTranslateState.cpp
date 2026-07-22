#include "states/KeyboardTranslateState.h"

KeyboardTranslateState::KeyboardTranslateState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
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
