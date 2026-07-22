#include "states/NavigationState.h"

NavigationState::NavigationState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

NavigationState::~NavigationState() = default;

void NavigationState::handle_key_press()
{
}

const char* NavigationState::get_name() const
{
    return "NavigationState";
}
