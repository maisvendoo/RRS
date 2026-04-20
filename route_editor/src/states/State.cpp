#include "states/State.h"

State::~State() = default;

void State::handle_key_press(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;
}
