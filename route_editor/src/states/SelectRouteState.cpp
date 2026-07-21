#include "states/SelectRouteState.h"

SelectRouteState::SelectRouteState(
    const vsg::ref_ptr<Mouse>& mouse,
    const vsg::ref_ptr<Keyboard>& keyboard
)
    : State(mouse, keyboard)
{
}

SelectRouteState::~SelectRouteState() = default;
