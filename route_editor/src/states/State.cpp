#include "states/State.h"

State::~State() = default;

void State::handle_key_press(vsg::KeyPressEvent& keyPress)
{
    (void)keyPress;
}

void State::handle_key_release(vsg::KeyReleaseEvent& keyRelease)
{
    (void)keyRelease;
}
