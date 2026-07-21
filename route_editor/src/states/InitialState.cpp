#include "states/InitialState.h"

InitialState::InitialState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

InitialState::~InitialState() = default;

void InitialState::handle_key_press(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;
}
